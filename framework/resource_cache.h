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

#include "resource_record.h"
#include "resource_replay.h"

#include "core/descriptor_set.h"
#include "core/descriptor_set_layout.h"
#include "core/framebuffer.h"

namespace vkb
{
class Device;

/**
 * @brief Cache Vulkan objects specific to a Vulkan device.
          Supports serialization and deserialization of cached resources. 
 */
class ResourceCache : public NonCopyable
{
  public:
	ResourceCache(Device &device);

	void warmup(const std::vector<uint8_t> &data);

	std::vector<uint8_t> serialize();

	void set_pipeline_cache(VkPipelineCache pipeline_cache);

	ShaderModule &request_shader_module(VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const ShaderVariant &shader_variant = {});

	PipelineLayout &request_pipeline_layout(const std::vector<ShaderModule *> &shader_modules);

	DescriptorSetLayout &request_descriptor_set_layout(const std::vector<ShaderResource> &set_resources);

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

	void clear_pipelines();

	void clear_framebuffers();

	void clear();

  private:
	Device &device;

	ResourceRecord recorder;

	ResourceReplay replayer;

	VkPipelineCache pipeline_cache{VK_NULL_HANDLE};

	std::unordered_map<std::size_t, ShaderModule> shader_modules;

	std::unordered_map<std::size_t, PipelineLayout> pipeline_layouts;

	std::unordered_map<std::size_t, DescriptorSetLayout> descriptor_set_layouts;

	std::unordered_map<std::size_t, RenderPass> render_passes;

	std::unordered_map<std::size_t, GraphicsPipeline> graphics_pipelines;

	std::unordered_map<std::size_t, ComputePipeline> compute_pipelines;

	std::unordered_map<std::size_t, DescriptorSet> descriptor_sets;

	std::unordered_map<std::size_t, Framebuffer> framebuffers;
};
}        // namespace vkb

namespace std
{
template <>
struct hash<vkb::ShaderSource>
{
	std::size_t operator()(const vkb::ShaderSource &shader_source) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, shader_source.get_id());

		return result;
	}
};

template <>
struct hash<vkb::ShaderVariant>
{
	std::size_t operator()(const vkb::ShaderVariant &shader_variant) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, shader_variant.get_id());

		return result;
	}
};

template <>
struct hash<vkb::ShaderModule>
{
	std::size_t operator()(const vkb::ShaderModule &shader_module) const
	{
		std::size_t result = 0;

		vkb::hash_combine(result, shader_module.get_id());

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
struct hash<vkb::ShaderResource>
{
	std::size_t operator()(const vkb::ShaderResource &shader_resource) const
	{
		std::size_t result = 0;

		if (shader_resource.type == vkb::ShaderResourceType::Input ||
		    shader_resource.type == vkb::ShaderResourceType::Output ||
		    shader_resource.type == vkb::ShaderResourceType::PushConstant ||
		    shader_resource.type == vkb::ShaderResourceType::SpecializationConstant)
		{
			return result;
		}

		vkb::hash_combine(result, shader_resource.set);
		vkb::hash_combine(result, shader_resource.binding);
		vkb::hash_combine(result, static_cast<std::underlying_type<vkb::ShaderResourceType>::type>(shader_resource.type));

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

		for (auto stage : graphics_state.get_pipeline_layout().get_stages())
		{
			vkb::hash_combine(result, stage->get_id());
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
