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

#pragma once

#include "common.h"

#include "command_pool.h"
#include "core/command_buffer.h"
#include "core/command_pool.h"
#include "core/descriptor_set.h"
#include "core/descriptor_set_layout.h"
#include "core/framebuffer.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/queue.h"
#include "core/render_pass.h"
#include "core/shader_module.h"
#include "core/swapchain.h"
#include "fence_pool.h"

#include "cache_resource.h"
#include "graphics_pipeline_state.h"
#include "render_frame.h"
#include "render_target.h"

namespace vkb
{
class Device : public NonCopyable
{
  public:
	Device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const std::vector<const char *> extensions = {}, const VkPhysicalDeviceFeatures &features = {});

	~Device();

	VkPhysicalDevice get_physical_device() const;

	VkDevice get_handle() const;

	VmaAllocator get_memory_allocator() const;

	const VkPhysicalDeviceProperties &get_properties() const;

	const Queue &get_queue(uint32_t queue_family_index, uint32_t queue_index);

	const Queue &get_queue_by_flags(VkQueueFlags queue_flags, uint32_t queue_index);

	const Queue &get_queue_by_present(uint32_t queue_index);

	/**
	 * @return The command pool
	 */
	CommandPool &get_command_pool()
	{
		return *command_pool;
	}

	/**
	 * @brief Requests a command buffer from the general command_pool
	 * @return A new command buffer
	 */
	CommandBuffer &request_command_buffer();

	/**
	 * @return The fence pool
	 */
	FencePool &get_fence_pool()
	{
		return *fence_pool;
	}

	/**
	 * @brief Requests a fence to the fence pool
	 * @return A vulkan fence
	 */
	VkFence request_fence();

	VkResult wait_idle();

	PipelineLayout &request_pipeline_layout(std::vector<ShaderModule> &&shader_modules);

	GraphicsPipeline &request_graphics_pipeline(GraphicsPipelineState &                   graphics_state,
	                                            const ShaderStageMap<SpecializationInfo> &specialization_infos);

	ComputePipeline &request_compute_pipeline(const PipelineLayout &    pipeline_layout,
	                                          const SpecializationInfo &specialization_info);

	DescriptorSet &request_descriptor_set(DescriptorSetLayout &                     descriptor_set_layout,
	                                      const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
	                                      const BindingMap<VkDescriptorImageInfo> & image_infos);

	RenderPass &request_render_pass(const std::vector<Attachment> &   attachments,
	                                const std::vector<LoadStoreInfo> &load_store_infos,
	                                const std::vector<SubpassInfo> &  subpasses);

	Framebuffer &request_framebuffer(const RenderTarget &render_target,
	                                 const RenderPass &  render_pass);

	/**
	 * @brief Deletes all framebuffers in the cache
	 */
	void clear_framebuffers();

  private:
	VkPhysicalDevice physical_device{VK_NULL_HANDLE};

	VkSurfaceKHR surface{VK_NULL_HANDLE};

	uint32_t queue_family_count{0};

	VkDevice handle{VK_NULL_HANDLE};

	VmaAllocator memory_allocator{VK_NULL_HANDLE};

	VkPhysicalDeviceProperties properties;

	std::vector<std::vector<Queue>> queues;

	/// A command pool associated to the primary queue
	std::unique_ptr<CommandPool> command_pool;

	/// A fence pool associated to the primary queue
	std::unique_ptr<FencePool> fence_pool;

	CacheResource<PipelineLayout> cache_pipeline_layouts;

	CacheResource<GraphicsPipeline> cache_graphics_pipelines;

	CacheResource<ComputePipeline> cache_compute_pipelines;

	CacheResource<DescriptorSet> cache_descriptor_sets;

	CacheResource<RenderPass> cache_render_passes;

	CacheResource<Framebuffer> cache_framebuffers;
};

}        // namespace vkb

namespace std
{
template <>
struct hash<vkb::Device>
{
	std::size_t operator()(const vkb::Device &device) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, device.get_handle());

		return result;
	}
};

template <>
struct hash<vkb::ShaderModule>
{
	std::size_t operator()(const vkb::ShaderModule &shader_module) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, shader_module.get_handle());

		return result;
	}
};

template <>
struct hash<vkb::DescriptorSetLayout>
{
	std::size_t operator()(const vkb::DescriptorSetLayout &descriptor_set_layout) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, descriptor_set_layout.get_handle());

		return result;
	}
};

template <>
struct hash<vkb::PipelineLayout>
{
	std::size_t operator()(const vkb::PipelineLayout &pipeline_layout) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, pipeline_layout.get_handle());

		return result;
	}
};

template <>
struct hash<vkb::RenderPass>
{
	std::size_t operator()(const vkb::RenderPass &render_pass) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, render_pass.get_handle());

		return result;
	}
};

template <>
struct hash<vkb::Attachment>
{
	std::size_t operator()(const vkb::Attachment &attachment) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, static_cast<std::underlying_type<VkFormat>::type>(attachment.format));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkSampleCountFlagBits>::type>(attachment.samples));

		return result;
	}
};

template <>
struct hash<vkb::LoadStoreInfo>
{
	std::size_t operator()(const vkb::LoadStoreInfo &load_store_info) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, static_cast<std::underlying_type<VkAttachmentLoadOp>::type>(load_store_info.load_op));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkAttachmentStoreOp>::type>(load_store_info.store_op));

		return result;
	}
};

template <>
struct hash<vkb::SubpassInfo>
{
	std::size_t operator()(const vkb::SubpassInfo &subpass_info) const
	{
		std::size_t result = 0;

		for (uint32_t output_attachment : subpass_info.output_attachments)
		{
			vkb::hash_combine(result, output_attachment);
		}

		for (uint32_t input_attachment : subpass_info.input_attachments)
		{
			vkb::hash_combine(result, input_attachment);
		}

		return result;
	}
};

template <>
struct hash<vkb::SpecializationInfo>
{
	std::size_t operator()(const vkb::SpecializationInfo &specialization_info) const
	{
		std::size_t result = 0;

		auto data = specialization_info.get_data();

		vkb::hash_combine(result, std::string(data.begin(), data.end()));

		for (auto &map_entry : specialization_info.get_map_entries())
		{
			vkb::hash_combine(result, map_entry);
		}

		return result;
	}
};

template <>
struct hash<VkSpecializationMapEntry>
{
	std::size_t operator()(const VkSpecializationMapEntry &specialization_map_entry) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, specialization_map_entry.constantID);
		vkb::hash_combine(result, specialization_map_entry.offset);
		vkb::hash_combine(result, specialization_map_entry.size);

		return result;
	}
};

template <>
struct hash<VkDescriptorBufferInfo>
{
	std::size_t operator()(const VkDescriptorBufferInfo &descriptor_buffer_info) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, descriptor_buffer_info.buffer);
		vkb::hash_combine(result, descriptor_buffer_info.range);
		vkb::hash_combine(result, descriptor_buffer_info.offset);

		return result;
	}
};

template <>
struct hash<VkDescriptorImageInfo>
{
	std::size_t operator()(const VkDescriptorImageInfo &descriptor_image_info) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, descriptor_image_info.imageView);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkImageLayout>::type>(descriptor_image_info.imageLayout));
		vkb::hash_combine(result, descriptor_image_info.sampler);

		return result;
	}
};

template <>
struct hash<VkVertexInputAttributeDescription>
{
	std::size_t operator()(const VkVertexInputAttributeDescription &vertex_attrib) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, vertex_attrib.binding);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkFormat>::type>(vertex_attrib.format));
		vkb::hash_combine(result, vertex_attrib.location);
		vkb::hash_combine(result, vertex_attrib.offset);

		return result;
	}
};

template <>
struct hash<VkVertexInputBindingDescription>
{
	std::size_t operator()(const VkVertexInputBindingDescription &vertex_binding) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, vertex_binding.binding);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkVertexInputRate>::type>(vertex_binding.inputRate));
		vkb::hash_combine(result, vertex_binding.stride);

		return result;
	}
};

template <>
struct hash<vkb::StencilOpState>
{
	std::size_t operator()(const vkb::StencilOpState &stencil) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, static_cast<std::underlying_type<VkCompareOp>::type>(stencil.compare_op));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.depth_fail_op));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.fail_op));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.pass_op));

		return result;
	}
};

template <>
struct hash<VkExtent2D>
{
	size_t operator()(const VkExtent2D &extent) const
	{
		size_t result = 0;

		vkb::hash_combine(result, extent.width);
		vkb::hash_combine(result, extent.height);

		return result;
	}
};

template <>
struct hash<VkOffset2D>
{
	size_t operator()(const VkOffset2D &offset) const
	{
		size_t result = 0;

		vkb::hash_combine(result, offset.x);
		vkb::hash_combine(result, offset.y);

		return result;
	}
};

template <>
struct hash<VkRect2D>
{
	size_t operator()(const VkRect2D &rect) const
	{
		size_t result = 0;

		vkb::hash_combine(result, rect.extent);
		vkb::hash_combine(result, rect.offset);

		return result;
	}
};

template <>
struct hash<VkViewport>
{
	size_t operator()(const VkViewport &viewport) const
	{
		size_t result = 0;

		vkb::hash_combine(result, viewport.width);
		vkb::hash_combine(result, viewport.height);
		vkb::hash_combine(result, viewport.maxDepth);
		vkb::hash_combine(result, viewport.minDepth);
		vkb::hash_combine(result, viewport.x);
		vkb::hash_combine(result, viewport.y);

		return result;
	}
};

template <>
struct hash<vkb::ColorBlendAttachmentState>
{
	std::size_t operator()(const vkb::ColorBlendAttachmentState &color_blend_attachment) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, static_cast<std::underlying_type<VkBlendOp>::type>(color_blend_attachment.alpha_blend_op));
		vkb::hash_combine(result, color_blend_attachment.blend_enable);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkBlendOp>::type>(color_blend_attachment.color_blend_op));
		vkb::hash_combine(result, color_blend_attachment.color_write_mask);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.dst_alpha_blend_factor));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.dst_color_blend_factor));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.src_alpha_blend_factor));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.src_color_blend_factor));

		return result;
	}
};

template <>
struct hash<vkb::RenderTarget>
{
	std::size_t operator()(const vkb::RenderTarget &render_target) const
	{
		std::size_t result = 0;

		for (auto &view : render_target.get_views())
		{
			vkb::hash_combine(result, view.get_handle());
		}

		return result;
	}
};

template <>
struct hash<vkb::GraphicsPipelineState>
{
	std::size_t operator()(const vkb::GraphicsPipelineState &graphics_state) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, graphics_state.get_pipeline_layout().get_handle());
		vkb::hash_combine(result, graphics_state.get_render_pass().get_handle());
		vkb::hash_combine(result, graphics_state.get_subpass_index());

		for (auto &stage : graphics_state.get_pipeline_layout().get_stages())
		{
			vkb::hash_combine(result, stage.get_handle());
		}

		// VkPipelineVertexInputStateCreateInfo
		for (auto &attribute : graphics_state.get_vertex_input_state().attributes)
		{
			vkb::hash_combine(result, attribute);
		}

		for (auto &binding : graphics_state.get_vertex_input_state().bindings)
		{
			vkb::hash_combine(result, binding);
		}

		// VkPipelineInputAssemblyStateCreateInfo

		vkb::hash_combine(result, graphics_state.get_input_assembly_state().primitive_restart_enable);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkPrimitiveTopology>::type>(graphics_state.get_input_assembly_state().topology));

		//VkPipelineViewportStateCreateInfo
		vkb::hash_combine(result, graphics_state.get_viewport_state().viewport_count);
		vkb::hash_combine(result, graphics_state.get_viewport_state().scissor_count);

		// VkPipelineRasterizationStateCreateInfo

		vkb::hash_combine(result, graphics_state.get_rasterization_state().cull_mode);
		vkb::hash_combine(result, graphics_state.get_rasterization_state().depth_bias_enable);
		vkb::hash_combine(result, graphics_state.get_rasterization_state().depth_clamp_enable);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkFrontFace>::type>(graphics_state.get_rasterization_state().front_face));
		vkb::hash_combine(result, static_cast<std::underlying_type<VkPolygonMode>::type>(graphics_state.get_rasterization_state().polygon_mode));
		vkb::hash_combine(result, graphics_state.get_rasterization_state().rasterizer_discard_enable);

		// VkPipelineMultisampleStateCreateInfo

		vkb::hash_combine(result, graphics_state.get_multisample_state().alpha_to_coverage_enable);
		vkb::hash_combine(result, graphics_state.get_multisample_state().alpha_to_one_enable);
		vkb::hash_combine(result, graphics_state.get_multisample_state().min_sample_shading);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkSampleCountFlagBits>::type>(graphics_state.get_multisample_state().rasterization_samples));
		vkb::hash_combine(result, graphics_state.get_multisample_state().sample_shading_enable);
		vkb::hash_combine(result, graphics_state.get_multisample_state().sample_mask);

		// VkPipelineDepthStencilStateCreateInfo

		vkb::hash_combine(result, graphics_state.get_depth_stencil_state().back);
		vkb::hash_combine(result, graphics_state.get_depth_stencil_state().depth_bounds_test_enable);
		vkb::hash_combine(result, static_cast<std::underlying_type<VkCompareOp>::type>(graphics_state.get_depth_stencil_state().depth_compare_op));
		vkb::hash_combine(result, graphics_state.get_depth_stencil_state().depth_test_enable);
		vkb::hash_combine(result, graphics_state.get_depth_stencil_state().depth_write_enable);
		vkb::hash_combine(result, graphics_state.get_depth_stencil_state().front);
		vkb::hash_combine(result, graphics_state.get_depth_stencil_state().stencil_test_enable);

		// VkPipelineColorBlendStateCreateInfo

		vkb::hash_combine(result, static_cast<std::underlying_type<VkLogicOp>::type>(graphics_state.get_color_blend_state().logic_op));
		vkb::hash_combine(result, graphics_state.get_color_blend_state().logic_op_enable);

		for (auto &attachment : graphics_state.get_color_blend_state().attachments)
		{
			vkb::hash_combine(result, attachment);
		}

		return result;
	}
};
}        // namespace std
