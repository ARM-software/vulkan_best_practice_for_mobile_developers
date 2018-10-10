/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "postprocess.h"

#include "core/pipeline.h"
#include "core/pipeline_layout.h"

namespace vkb
{
PostProcess::PostProcess(Context *ctx) :
    context{ctx}
{
	assert(ctx && "Context should be valid");
}

PostProcess::~PostProcess()
{
	teardown_postprocess();
}

void PostProcess::teardown_postprocess()
{
	// Destroy per-pass resources
	for (auto &ppc_resources : pp_resources)
	{
		for (auto &ppp_resources : ppc_resources)
		{
			for (auto &image : ppp_resources.images)
			{
				image.cleanup();
			}

			if (ppp_resources.framebuffer != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer(context->device, ppp_resources.framebuffer, nullptr);
			}
		}
	}

	// Destroy common resources across post-process chains
	if (!pp_resources.empty())
	{
		auto &ppc_resources = pp_resources.front();

		for (auto &ppp_resources : ppc_resources)
		{
			if (ppp_resources.desc_set_layout != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorSetLayout(context->device, ppp_resources.desc_set_layout, nullptr);
			}

			if (ppp_resources.pipeline != VK_NULL_HANDLE)
			{
				vkDestroyPipeline(context->device, ppp_resources.pipeline, nullptr);
			}

			if (ppp_resources.pipeline_layout != VK_NULL_HANDLE)
			{
				vkDestroyPipelineLayout(context->device, ppp_resources.pipeline_layout, nullptr);
			}

			if (ppp_resources.render_pass != VK_NULL_HANDLE)
			{
				vkDestroyRenderPass(context->device, ppp_resources.render_pass, nullptr);
			}
		}
	}

	pp_resources.clear();
	passes.clear();
	input_images.clear();

	// Destroy common resources
	if (desc_pool != VK_NULL_HANDLE)
	{
		// Destroying a descriptor pool also destroys the allocated
		// descriptor sets
		vkDestroyDescriptorPool(context->device, desc_pool, nullptr);
		desc_pool = VK_NULL_HANDLE;
	}

	if (sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(context->device, sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}
};

bool PostProcess::init(std::vector<ChainInputImages> input_images_, std::vector<PostProcessPass> passes_)
{
	if (input_images_.empty())
	{
		LOGE("Post-processing needs at least one input image.");
		return false;
	}
	input_images = std::move(input_images_);
	passes       = std::move(passes_);

	// Create a sampler, which will be common to all images
	VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sampler_info.addressModeU        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeV        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeW        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.maxLod              = 1;
	sampler_info.minFilter           = VK_FILTER_LINEAR;
	sampler_info.magFilter           = VK_FILTER_LINEAR;

	VK_CHECK(vkCreateSampler(context->device, &sampler_info, nullptr, &sampler));

	pp_resources.resize(context->swapchain.frames.size());

	// Create a descriptor set layout for the input images
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (size_t i = 0; i < input_images[i].size(); i++)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding                      = static_cast<uint32_t>(i);
		binding.descriptorCount              = 1;
		binding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.pImmutableSamplers           = &sampler;

		bindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	desc_layout_info.bindingCount                    = static_cast<uint32_t>(bindings.size());
	desc_layout_info.pBindings                       = bindings.data();

	VkDescriptorSetLayout desc_layout = VK_NULL_HANDLE;
	VK_CHECK(vkCreateDescriptorSetLayout(context->device, &desc_layout_info, nullptr, &desc_layout));

	// We run a first pass in which we initialize most of the resources needed
	// for postprocessing, except for descriptor sets, which will require a second pass
	auto end       = passes.end();
	auto last_pass = end - 1;
	for (auto it = passes.begin(); it != end; ++it)
	{
		auto &ppp = *it;

		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference>   attachment_refs;

		if (it == last_pass)
		{
			// The last pass will write to the swapchain image
			ppp.relative_width  = 1.0f;
			ppp.relative_height = 1.0f;
			ppp.image_formats   = {context->swapchain.format};
		}

		// Create a render pass for each pass
		for (uint32_t i = 0; i < ppp.image_formats.size(); i++)
		{
			VkAttachmentDescription attachment = {};
			attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout             = (it == last_pass) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.format                  = ppp.image_formats[i];
			attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
			attachments.push_back(attachment);

			VkAttachmentReference attachment_ref = {};
			attachment_ref.attachment            = i;
			attachment_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment_refs.push_back(attachment_ref);
		}

		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.dependencyFlags     = VK_DEPENDENCY_BY_REGION_BIT;
		subpass_dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass          = 0;
		subpass_dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.srcAccessMask       = 0;
		subpass_dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(attachment_refs.size());
		subpass.pColorAttachments    = attachment_refs.data();

		VkRenderPassCreateInfo rp_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		rp_info.attachmentCount        = static_cast<uint32_t>(attachments.size());
		rp_info.pAttachments           = attachments.data();
		rp_info.subpassCount           = 1;
		rp_info.pSubpasses             = &subpass;
		rp_info.dependencyCount        = (it == last_pass) ? 1 : 0;
		rp_info.pDependencies          = &subpass_dependency;

		VkRenderPass render_pass = VK_NULL_HANDLE;
		VK_CHECK(vkCreateRenderPass(context->device, &rp_info, nullptr, &render_pass));

		// Create a pipeline for the pass
		//auto &pipeline_layout = create_pipeline_layout(*context->vk_device, "shaders/postprocessing.vert", ppp.fragment_shader_path.c_str());

		GraphicsPipelineState graphics_state = {};
		/*
		graphics_state.render_pass           = render_pass;

		graphics_state.input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		graphics_state.multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		graphics_state.rasterization_state.polygonMode        = VK_POLYGON_MODE_FILL;
		graphics_state.rasterization_state.cullMode           = VK_CULL_MODE_NONE;
		graphics_state.rasterization_state.frontFace          = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		graphics_state.rasterization_state.lineWidth          = 1.0f;

		// Our attachment will write to all color channels, but no blending is enabled
		VkPipelineColorBlendAttachmentState blend_attachment{};
		blend_attachment.blendEnable                     = VK_FALSE;
		blend_attachment.colorWriteMask                  = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		graphics_state.color_blend_state.attachmentCount = 1;
		graphics_state.color_blend_state.pAttachments    = &blend_attachment;

		// We will have one viewport and scissor box
		graphics_state.viewport_state.viewportCount = 1;
		graphics_state.viewport_state.scissorCount  = 1;

		// Specify that these states will be dynamic, i.e. not part of pipeline state object
		static const VkDynamicState dynamics[] = {
		    VK_DYNAMIC_STATE_VIEWPORT,
		    VK_DYNAMIC_STATE_SCISSOR,
		};
		VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
		dynamic.pDynamicStates    = dynamics;
		dynamic.dynamicStateCount = 2;

		graphics_state.dynamic_state = dynamic;
		*/
		GraphicsPipeline *pipeline;        //             = pipeline_layout->request_graphics_pipeline(graphics_state, {});

		//(render_pass, pipeline_layout, ppp.fragment_shader_path.c_str(), static_cast<uint32_t>(attachments.size()));

		// Create the actual images and framebuffers needed for the pass,
		// one for each frame that can be in flight simultaneously
		for (uint32_t i = 0; i < context->swapchain.frames.size(); i++)
		{
			// Compute the absolute width and height for the images
			uint32_t width  = static_cast<uint32_t>(std::round(ppp.relative_width * context->swapchain.extent.width));
			uint32_t height = static_cast<uint32_t>(std::round(ppp.relative_height * context->swapchain.extent.height));

			PostProcessPassResources ppp_resources = {};
			ppp_resources.desc                     = ppp;
			ppp_resources.render_pass              = render_pass;
			//ppp_resources.pipeline_layout          = pipeline_layout.get_handle();
			ppp_resources.pipeline        = pipeline->get_handle();
			ppp_resources.desc_set_layout = desc_layout;

			// Gather the image views for the framebuffer
			std::vector<VkImageView> fb_images;
			if (it == last_pass)
			{
				// For the last pass we use the swapchain images as outputs
				fb_images = {context->swapchain.frames[i].color_attachment.view};
			}
			else
			{
				// Framebuffer images need to be created for passes in the middle
				for (auto format : ppp.image_formats)
				{
					auto image = Image(context->vk_device, width, height, VK_NULL_HANDLE, format,
					                   VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
					ppp_resources.images.push_back(image);
					fb_images.push_back(image.view);
				}
			}

			// Create the framebuffer
			VkFramebufferCreateInfo fb_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			fb_info.width                   = width;
			fb_info.height                  = height;
			fb_info.renderPass              = render_pass;
			fb_info.layers                  = 1;
			fb_info.attachmentCount         = static_cast<uint32_t>(fb_images.size());
			fb_info.pAttachments            = fb_images.data();

			VkFramebuffer framebuffer = VK_NULL_HANDLE;
			VK_CHECK(vkCreateFramebuffer(context->device, &fb_info, nullptr, &framebuffer));
			ppp_resources.framebuffer = framebuffer;

			pp_resources[i].push_back(ppp_resources);
		}

		if (it != last_pass)
		{
			// Prepare the descriptor set layout for the next pass
			bindings.clear();
			for (size_t i = 0; i < ppp.image_formats.size(); i++)
			{
				VkDescriptorSetLayoutBinding binding = {};
				binding.binding                      = static_cast<uint32_t>(i);
				binding.descriptorCount              = 1;
				binding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				binding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
				binding.pImmutableSamplers           = &sampler;

				bindings.push_back(binding);
			}

			desc_layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
			desc_layout_info.pBindings    = bindings.data();

			VK_CHECK(vkCreateDescriptorSetLayout(context->device, &desc_layout_info, nullptr, &desc_layout));
		}
	}

	// We now have out descriptor set layouts, but in order to allocate descriptor sets
	// we need to allocate a pool
	size_t image_count = 0;
	for (auto &ppp : passes)
	{
		image_count += ppp.image_formats.size();
	}
	image_count *= context->swapchain.frames.size();

	size_t desc_set_count = passes.size() * context->swapchain.frames.size();

	VkDescriptorPoolSize pool_size;
	pool_size.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_size.descriptorCount = static_cast<uint32_t>(image_count);

	VkDescriptorPoolCreateInfo desc_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	desc_pool_info.poolSizeCount              = 1;
	desc_pool_info.pPoolSizes                 = &pool_size;
	desc_pool_info.maxSets                    = static_cast<uint32_t>(desc_set_count);

	VK_CHECK(vkCreateDescriptorPool(context->device, &desc_pool_info, nullptr, &desc_pool));

	// We run a second pass to allocate and fill descriptor sets. This second pass
	// iterates through resources differently than before (the outer loop is now on per-frame
	// resources, while the inner loop is on passes).
	// This is necessary because for each frame we have to pass the descriptor set containing
	// the output images from a pass to the following one.
	for (size_t i = 0; i < pp_resources.size(); i++)
	{
		VkDescriptorSet desc_set = VK_NULL_HANDLE;

		// Create the first descriptor set for the inputs
		VkDescriptorSetAllocateInfo desc_set_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		desc_set_info.descriptorPool              = desc_pool;
		desc_set_info.descriptorSetCount          = 1;
		desc_set_info.pSetLayouts                 = &pp_resources[i][0].desc_set_layout;

		VK_CHECK(vkAllocateDescriptorSets(context->device, &desc_set_info, &desc_set));

		std::vector<VkDescriptorImageInfo> image_infos;
		for (auto &input_image : input_images[i])
		{
			VkDescriptorImageInfo image_info = {};
			image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView             = input_image.view;
			image_infos.push_back(image_info);
		}

		VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		descriptor_write.descriptorCount      = static_cast<uint32_t>(image_infos.size());
		descriptor_write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.dstBinding           = 0;
		descriptor_write.dstSet               = desc_set;
		descriptor_write.pImageInfo           = image_infos.data();

		vkUpdateDescriptorSets(context->device, 1, &descriptor_write, 0, nullptr);

		auto end       = pp_resources[i].end();
		auto last_pass = end - 1;
		for (auto it = pp_resources[i].begin(); it != end; ++it)
		{
			auto &ppp_resources    = *it;
			ppp_resources.desc_set = desc_set;

			if (it != last_pass)
			{
				// Create a descriptor set for the outputs of this pass, unless
				// this is the last pass
				desc_set_info.descriptorPool     = desc_pool;
				desc_set_info.descriptorSetCount = 1;
				desc_set_info.pSetLayouts        = &ppp_resources.desc_set_layout;

				VK_CHECK(vkAllocateDescriptorSets(context->device, &desc_set_info, &desc_set));

				image_infos.clear();
				for (auto &image : ppp_resources.images)
				{
					VkDescriptorImageInfo image_info = {};
					image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					image_info.imageView             = image.view;
					image_infos.push_back(image_info);
				}

				descriptor_write.descriptorCount = static_cast<uint32_t>(image_infos.size());
				descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_write.dstBinding      = 0;
				descriptor_write.dstSet          = desc_set;
				descriptor_write.pImageInfo      = image_infos.data();

				vkUpdateDescriptorSets(context->device, 1, &descriptor_write, 0, nullptr);
			}
		}
	}

	return true;
}

void PostProcess::on_resize(std::vector<ChainInputImages> input_images_)
{
	// Re-initialize postprocessing with the new set of images
	std::vector<PostProcessPass> stored_passes = std::move(passes);
	teardown_postprocess();
	init(std::move(input_images_), std::move(stored_passes));
}

void PostProcess::run(uint32_t swapchain_index)
{
	auto &ppc_resources    = pp_resources[swapchain_index];
	auto &ppc_input_images = input_images[swapchain_index];

	/// Render to this backbuffer.
	Frame &framebuffer = context->swapchain.frames[swapchain_index];

	/// Allocate or re-use a primary command buffer.
	VkCommandBuffer cmd = framebuffer.request_command_buffer(true);

	// We will only submit this once before it's recycled
	VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &begin_info);

	// Run each post-process pass
	for (auto &ppp_resources : ppc_resources)
	{
		std::vector<VkClearValue> clear_values(ppp_resources.desc.image_formats.size(), VkClearValue());
		clear_values[0].color = {0.3f, 0.0f, 0.0f, 1.0f};

		uint32_t width  = static_cast<uint32_t>(std::round(ppp_resources.desc.relative_width * context->swapchain.extent.width));
		uint32_t height = static_cast<uint32_t>(std::round(ppp_resources.desc.relative_height * context->swapchain.extent.height));

		// Begin the render pass
		VkRenderPassBeginInfo rp_begin    = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		rp_begin.renderPass               = ppp_resources.render_pass;
		rp_begin.framebuffer              = ppp_resources.framebuffer;
		rp_begin.renderArea.extent.width  = width;
		rp_begin.renderArea.extent.height = height;
		rp_begin.clearValueCount          = static_cast<uint32_t>(clear_values.size());
		rp_begin.pClearValues             = clear_values.data();

		vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

		// Bind the pipeline for this pass
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ppp_resources.pipeline);

		// Set up dynamic state (viewport and scissor)
		VkViewport vp = {};
		vp.width      = float(width);
		vp.height     = float(height);
		vp.minDepth   = 0.0f;
		vp.maxDepth   = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &vp);

		VkRect2D scissor      = {};
		scissor.extent.width  = width;
		scissor.extent.height = height;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		// Bind the descriptor set with input images for this pass
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ppp_resources.pipeline_layout, 0, 1, &ppp_resources.desc_set, 0, nullptr);

		vkCmdDraw(cmd, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmd);

		for (auto &image : ppp_resources.images)
		{
			// Add a barrier to transition the images so we can read from them
			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel            = 0;
			subresource_range.levelCount              = 1;
			subresource_range.baseArrayLayer          = 0;
			subresource_range.layerCount              = 1;

			VkImageMemoryBarrier image_barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
			image_barrier.image                = image.handle;
			image_barrier.oldLayout            = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			image_barrier.newLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_barrier.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			image_barrier.dstAccessMask        = VK_ACCESS_SHADER_READ_BIT;
			image_barrier.subresourceRange     = subresource_range;
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			                     0, nullptr, 0, nullptr, 1, &image_barrier);
		}
	}

	// Complete the command buffer
	VK_CHECK(vkEndCommandBuffer(cmd));

	// Submit our post-processing command buffer to the queue
	if (framebuffer.properties->swapchain_release_semaphore == VK_NULL_HANDLE)
	{
		VkSemaphore           release_semaphore;
		VkSemaphoreCreateInfo semaphore_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
		VK_CHECK(vkCreateSemaphore(context->device, &semaphore_info, nullptr, &release_semaphore));
		framebuffer.properties->swapchain_release_semaphore = release_semaphore;
	}

	VkSubmitInfo info       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	info.commandBufferCount = 1;
	info.pCommandBuffers    = &cmd;

	const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	info.waitSemaphoreCount               = 1;
	info.pWaitSemaphores                  = &framebuffer.properties->postprocessing_semaphore;
	info.pWaitDstStageMask                = &wait_stage;
	info.signalSemaphoreCount             = 1;
	info.pSignalSemaphores                = &framebuffer.properties->swapchain_release_semaphore;

	VK_CHECK(vkQueueSubmit(context->queue, 1, &info, framebuffer.properties->queue_submit_fence));
}

}        // namespace vkb
