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

#include "graphics_pipeline_state.h"

bool operator==(const VkVertexInputAttributeDescription &lhs, const VkVertexInputAttributeDescription &rhs)
{
	return std::tie(lhs.binding, lhs.format, lhs.location, lhs.offset) == std::tie(rhs.binding, rhs.format, rhs.location, rhs.offset);
}

bool operator==(const VkVertexInputBindingDescription &lhs, const VkVertexInputBindingDescription &rhs)
{
	return std::tie(lhs.binding, lhs.inputRate, lhs.stride) == std::tie(rhs.binding, rhs.inputRate, rhs.stride);
}

bool operator==(const vkb::ColorBlendAttachmentState &lhs, const vkb::ColorBlendAttachmentState &rhs)
{
	return std::tie(lhs.alpha_blend_op, lhs.blend_enable, lhs.color_blend_op, lhs.color_write_mask, lhs.dst_alpha_blend_factor, lhs.dst_color_blend_factor, lhs.src_alpha_blend_factor, lhs.src_color_blend_factor) ==
	       std::tie(rhs.alpha_blend_op, rhs.blend_enable, rhs.color_blend_op, rhs.color_write_mask, rhs.dst_alpha_blend_factor, rhs.dst_color_blend_factor, rhs.src_alpha_blend_factor, rhs.src_color_blend_factor);
}

bool operator!=(const vkb::StencilOpState &lhs, const vkb::StencilOpState &rhs)
{
	return std::tie(lhs.compare_op, lhs.depth_fail_op, lhs.fail_op, lhs.pass_op) != std::tie(rhs.compare_op, rhs.depth_fail_op, rhs.fail_op, rhs.pass_op);
}

bool operator!=(const vkb::VertexInputState &lhs, const vkb::VertexInputState &rhs)
{
	return lhs.attributes != rhs.attributes || lhs.bindings != rhs.bindings;
}

bool operator!=(const vkb::InputAssemblyState &lhs, const vkb::InputAssemblyState &rhs)
{
	return std::tie(lhs.primitive_restart_enable, lhs.topology) != std::tie(rhs.primitive_restart_enable, rhs.topology);
}

bool operator!=(const vkb::RasterizationState &lhs, const vkb::RasterizationState &rhs)
{
	return std::tie(lhs.cull_mode, lhs.depth_bias_enable, lhs.depth_clamp_enable, lhs.front_face, lhs.front_face, lhs.polygon_mode, lhs.rasterizer_discard_enable) !=
	       std::tie(rhs.cull_mode, rhs.depth_bias_enable, rhs.depth_clamp_enable, rhs.front_face, rhs.front_face, rhs.polygon_mode, rhs.rasterizer_discard_enable);
}

bool operator!=(const vkb::ViewportState &lhs, const vkb::ViewportState &rhs)
{
	return lhs.viewport_count != rhs.viewport_count || lhs.scissor_count != rhs.scissor_count;
}

bool operator!=(const vkb::MultisampleState &lhs, const vkb::MultisampleState &rhs)
{
	return std::tie(lhs.alpha_to_coverage_enable, lhs.alpha_to_one_enable, lhs.min_sample_shading, lhs.rasterization_samples, lhs.sample_mask, lhs.sample_shading_enable) !=
	       std::tie(rhs.alpha_to_coverage_enable, rhs.alpha_to_one_enable, rhs.min_sample_shading, rhs.rasterization_samples, rhs.sample_mask, rhs.sample_shading_enable);
}

bool operator!=(const vkb::DepthStencilState &lhs, const vkb::DepthStencilState &rhs)
{
	return std::tie(lhs.depth_bounds_test_enable, lhs.depth_compare_op, lhs.depth_test_enable, lhs.depth_write_enable, lhs.stencil_test_enable) !=
	           std::tie(rhs.depth_bounds_test_enable, rhs.depth_compare_op, rhs.depth_test_enable, rhs.depth_write_enable, rhs.stencil_test_enable) ||
	       lhs.back != rhs.back || lhs.front != rhs.front;
}

bool operator!=(const vkb::ColorBlendState &lhs, const vkb::ColorBlendState &rhs)
{
	return std::tie(lhs.logic_op, lhs.logic_op_enable) != std::tie(rhs.logic_op, rhs.logic_op_enable) ||
	       lhs.attachments.size() != rhs.attachments.size() ||
	       !std::equal(lhs.attachments.begin(), lhs.attachments.end(), rhs.attachments.begin(),
	                   [](const vkb::ColorBlendAttachmentState &lhs, const vkb::ColorBlendAttachmentState &rhs) {
		                   return lhs == rhs;
	                   });
}

namespace vkb
{
void SpecializationInfo::set_constant(uint32_t constant_id, const std::vector<uint8_t> &value)
{
	VkSpecializationMapEntry specialization_entry;

	specialization_entry.constantID = constant_id;
	specialization_entry.size       = value.size();
	specialization_entry.offset     = data.size();

	map_entries.push_back(specialization_entry);

	data.insert(data.end(), value.begin(), value.end());
}

const std::vector<uint8_t> &SpecializationInfo::get_data() const
{
	return data;
}

const std::vector<VkSpecializationMapEntry> &SpecializationInfo::get_map_entries() const
{
	return map_entries;
}

const VkSpecializationInfo &SpecializationInfo::get_handle() const
{
	handle.dataSize      = data.size();
	handle.pData         = data.data();
	handle.mapEntryCount = map_entries.size();
	handle.pMapEntries   = map_entries.data();

	return handle;
}

void GraphicsPipelineState::reset()
{
	clear_dirty();

	pipeline_layout = nullptr;

	render_pass = nullptr;

	vertex_input_sate = {};

	input_assembly_state = {};

	rasterization_state = {};

	multisample_state = {};

	depth_stencil_state = {};

	color_blend_state = {};

	subpass_index = {0U};
}

void GraphicsPipelineState::set_pipeline_layout(PipelineLayout &pipeline_layout)
{
	if (this->pipeline_layout)
	{
		if (this->pipeline_layout->get_handle() != pipeline_layout.get_handle())
		{
			this->pipeline_layout = &pipeline_layout;

			dirty = true;
		}
	}
	else
	{
		this->pipeline_layout = &pipeline_layout;

		dirty = true;
	}
}

void GraphicsPipelineState::set_render_pass(const RenderPass &render_pass)
{
	if (this->render_pass)
	{
		if (this->render_pass->get_handle() != render_pass.get_handle())
		{
			this->render_pass = &render_pass;

			dirty = true;
		}
	}
	else
	{
		this->render_pass = &render_pass;

		dirty = true;
	}
}

void GraphicsPipelineState::set_vertex_input_state(const VertexInputState &vertex_input_sate)
{
	if (this->vertex_input_sate != vertex_input_sate)
	{
		this->vertex_input_sate = vertex_input_sate;

		dirty = true;
	}
}

void GraphicsPipelineState::set_input_assembly_state(const InputAssemblyState &input_assembly_state)
{
	if (this->input_assembly_state != input_assembly_state)
	{
		this->input_assembly_state = input_assembly_state;

		dirty = true;
	}
}

void GraphicsPipelineState::set_rasterization_state(const RasterizationState &rasterization_state)
{
	if (this->rasterization_state != rasterization_state)
	{
		this->rasterization_state = rasterization_state;

		dirty = true;
	}
}

void GraphicsPipelineState::set_viewport_state(const ViewportState &viewport_state)
{
	if (this->viewport_state != viewport_state)
	{
		this->viewport_state = viewport_state;

		dirty = true;
	}
}

void GraphicsPipelineState::set_multisample_state(const MultisampleState &multisample_state)
{
	if (this->multisample_state != multisample_state)
	{
		this->multisample_state = multisample_state;

		dirty = true;
	}
}

void GraphicsPipelineState::set_depth_stencil_state(const DepthStencilState &depth_stencil_state)
{
	if (this->depth_stencil_state != depth_stencil_state)
	{
		this->depth_stencil_state = depth_stencil_state;

		dirty = true;
	}
}

void GraphicsPipelineState::set_color_blend_state(const ColorBlendState &color_blend_state)
{
	if (this->color_blend_state != color_blend_state)
	{
		this->color_blend_state = color_blend_state;

		dirty = true;
	}
}

void GraphicsPipelineState::set_subpass_index(uint32_t subpass_index)
{
	if (this->subpass_index != subpass_index)
	{
		this->subpass_index = subpass_index;

		dirty = true;
	}
}

const PipelineLayout &GraphicsPipelineState::get_pipeline_layout() const
{
	assert(pipeline_layout && "Graphics state Pipeline layout is not set");
	return *pipeline_layout;
}

const RenderPass &GraphicsPipelineState::get_render_pass() const
{
	return *render_pass;
}

const VertexInputState &GraphicsPipelineState::get_vertex_input_state() const
{
	return vertex_input_sate;
}

const InputAssemblyState &GraphicsPipelineState::get_input_assembly_state() const
{
	return input_assembly_state;
}

const RasterizationState &GraphicsPipelineState::get_rasterization_state() const
{
	return rasterization_state;
}

const ViewportState &GraphicsPipelineState::get_viewport_state() const
{
	return viewport_state;
}

const MultisampleState &GraphicsPipelineState::get_multisample_state() const
{
	return multisample_state;
}

const DepthStencilState &GraphicsPipelineState::get_depth_stencil_state() const
{
	return depth_stencil_state;
}

const ColorBlendState &GraphicsPipelineState::get_color_blend_state() const
{
	return color_blend_state;
}

uint32_t GraphicsPipelineState::get_subpass_index() const
{
	return subpass_index;
}

bool GraphicsPipelineState::is_dirty() const
{
	return dirty;
}

void GraphicsPipelineState::clear_dirty()
{
	dirty = false;
}
}        // namespace vkb
