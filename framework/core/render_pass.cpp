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

#include "core/render_pass.h"

#include <numeric>

#include "device.h"
#include "rendering/render_target.h"

namespace vkb
{
VkRenderPass RenderPass::get_handle() const
{
	return handle;
}

RenderPass::RenderPass(Device &device, const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses) :
    device{device}
{
	uint32_t depth_stencil_attachment{VK_ATTACHMENT_UNUSED};

	std::vector<VkAttachmentDescription> attachment_descriptions;

	for (uint32_t i = 0U; i < attachments.size(); ++i)
	{
		VkAttachmentDescription attachment{};

		attachment.format  = attachments[i].format;
		attachment.samples = attachments[i].samples;

		if (i < load_store_infos.size())
		{
			attachment.loadOp         = load_store_infos[i].load_op;
			attachment.storeOp        = load_store_infos[i].store_op;
			attachment.stencilLoadOp  = load_store_infos[i].load_op;
			attachment.stencilStoreOp = load_store_infos[i].store_op;
		}

		if (is_depth_stencil_format(attachment.format))
		{
			depth_stencil_attachment = i;
		}

		attachment_descriptions.push_back(std::move(attachment));
	}

	std::vector<VkSubpassDescription> subpass_descriptions;
	subpass_descriptions.reserve(subpasses.size());

	std::vector<VkAttachmentReference> input_attachments;
	std::vector<VkAttachmentReference> color_attachments;
	std::vector<VkAttachmentReference> depth_stencil_attachments;

	// Reserve space for the vectors
	auto input_attachments_count = std::accumulate(std::begin(subpasses), std::end(subpasses), 0,
	                                               [](size_t count, auto &subpass) { return to_u32(count) + to_u32(subpass.input_attachments.size()); });
	input_attachments.reserve(input_attachments_count);
	auto color_attachments_count = subpasses.size();
	color_attachments.reserve(color_attachments_count);
	auto ds_attachments_count = depth_stencil_attachment == VK_ATTACHMENT_UNUSED ? 0 : subpasses.size();
	depth_stencil_attachments.reserve(ds_attachments_count);

	for (auto &subpass : subpasses)
	{
		VkSubpassDescription subpass_description{};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (uint32_t k : subpass.input_attachments)
		{
			if (is_depth_stencil_format(attachment_descriptions[k].format))
			{
				input_attachments.push_back({k, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
			}
			else
			{
				input_attachments.push_back({k, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
			}

			if (!subpass_description.pInputAttachments)
			{
				subpass_description.pInputAttachments = &(*input_attachments.rbegin());
			}
		}

		subpass_description.inputAttachmentCount = to_u32(subpass.input_attachments.size());

		for (auto output_attachment : subpass.output_attachments)
		{
			if (output_attachment == depth_stencil_attachment)
			{
				continue;
			}

			color_attachments.push_back({output_attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

			if (!subpass_description.pColorAttachments)
			{
				subpass_description.pColorAttachments = &(*color_attachments.rbegin());
			}
		}

		if (depth_stencil_attachment != VK_ATTACHMENT_UNUSED)
		{
			depth_stencil_attachments.push_back({depth_stencil_attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

			subpass_description.pDepthStencilAttachment = &(*depth_stencil_attachments.rbegin());
		}

		subpass_description.colorAttachmentCount = to_u32(subpass.output_attachments.size());

		subpass_descriptions.push_back(subpass_description);
	}

	if (subpasses.empty())
	{
		VkSubpassDescription subpass_description{};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (uint32_t k = 0U; k < attachment_descriptions.size(); ++k)
		{
			if (k == depth_stencil_attachment)
			{
				continue;
			}

			color_attachments.push_back({k, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

			if (!subpass_description.pColorAttachments)
			{
				subpass_description.pColorAttachments = &(*color_attachments.rbegin());
			}
		}

		if (depth_stencil_attachment != VK_ATTACHMENT_UNUSED)
		{
			depth_stencil_attachments.push_back({depth_stencil_attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

			subpass_description.pDepthStencilAttachment = &(*depth_stencil_attachments.rbegin());
		}

		subpass_descriptions.push_back(subpass_description);
	}

	// Make the initial layout same as in the first subpass using that attachment
	for (auto &subpass : subpass_descriptions)
	{
		for (uint32_t k = 0U; k < subpass.colorAttachmentCount; ++k)
		{
			auto reference = subpass.pColorAttachments[k];
			// Set it only if not defined yet
			if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachment_descriptions[reference.attachment].initialLayout = reference.layout;
			}
		}

		for (uint32_t k = 0U; k < subpass.inputAttachmentCount; ++k)
		{
			auto reference = subpass.pInputAttachments[k];
			// Set it only if not defined yet
			if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachment_descriptions[reference.attachment].initialLayout = reference.layout;
			}
		}

		if (subpass.pDepthStencilAttachment)
		{
			auto reference = *subpass.pDepthStencilAttachment;
			// Set it only if not defined yet
			if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				attachment_descriptions[reference.attachment].initialLayout = reference.layout;
			}
		}
	}

	// Swapchain image final layout should be present
	attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Make the final layout same as the last subpass layout
	{
		VkSubpassDescription &subpass = subpass_descriptions.back();

		for (uint32_t k = 0U; k < subpass.colorAttachmentCount; ++k)
		{
			VkAttachmentReference reference = subpass.pColorAttachments[k];

			attachment_descriptions[reference.attachment].finalLayout = reference.layout;
		}

		for (uint32_t k = 0U; k < subpass.inputAttachmentCount; ++k)
		{
			VkAttachmentReference reference = subpass.pInputAttachments[k];

			attachment_descriptions[reference.attachment].finalLayout = reference.layout;

			// Do not use depth attachment if used as input
			if (reference.attachment == depth_stencil_attachment)
			{
				subpass.pDepthStencilAttachment = nullptr;
			}
		}

		if (subpass.pDepthStencilAttachment)
		{
			VkAttachmentReference reference = *subpass.pDepthStencilAttachment;

			attachment_descriptions[reference.attachment].finalLayout = reference.layout;
		}
	}

	// Set subpass dependencies
	auto subpass_count = to_u32(subpass_descriptions.size());

	std::vector<VkSubpassDependency> dependencies;
	if (subpass_count > 1)
	{
		dependencies.resize(subpass_count - 1);

		for (uint32_t i = 0; i < dependencies.size(); ++i)
		{
			// Transition input attachments from color attachment to shader read
			dependencies[i].srcSubpass      = i;
			dependencies[i].dstSubpass      = i + 1;
			dependencies[i].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[i].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[i].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[i].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
			dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
	}

	// Create render pass
	VkRenderPassCreateInfo create_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

	create_info.attachmentCount = to_u32(attachment_descriptions.size());
	create_info.pAttachments    = attachment_descriptions.data();
	create_info.subpassCount    = subpass_count;
	create_info.pSubpasses      = subpass_descriptions.data();
	create_info.dependencyCount = to_u32(dependencies.size());
	create_info.pDependencies   = dependencies.data();

	auto result = vkCreateRenderPass(device.get_handle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create RenderPass"};
	}
}        // namespace vkb

RenderPass::RenderPass(RenderPass &&other) :
    device{other.device},
    handle{other.handle}
{
	other.handle = VK_NULL_HANDLE;
}

RenderPass::~RenderPass()
{
	// Destroy render pass
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device.get_handle(), handle, nullptr);
	}
}
}        // namespace vkb
