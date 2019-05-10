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

#include "core/pipeline_layout.h"
#include "core/render_pass.h"

namespace vkb
{
struct VertexInputState
{
	std::vector<VkVertexInputBindingDescription> bindings;

	std::vector<VkVertexInputAttributeDescription> attributes;
};

struct InputAssemblyState
{
	VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

	VkBool32 primitive_restart_enable{VK_FALSE};
};

struct RasterizationState
{
	VkBool32 depth_clamp_enable{VK_FALSE};

	VkBool32 rasterizer_discard_enable{VK_FALSE};

	VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};

	VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};

	VkFrontFace front_face{VK_FRONT_FACE_COUNTER_CLOCKWISE};

	VkBool32 depth_bias_enable{VK_FALSE};
};

struct ViewportState
{
	uint32_t viewport_count{1};

	uint32_t scissor_count{1};
};

struct MultisampleState
{
	VkSampleCountFlagBits rasterization_samples{VK_SAMPLE_COUNT_1_BIT};

	VkBool32 sample_shading_enable{VK_FALSE};

	float min_sample_shading{0.0f};

	VkSampleMask sample_mask{0};

	VkBool32 alpha_to_coverage_enable{VK_FALSE};

	VkBool32 alpha_to_one_enable{VK_FALSE};
};

struct StencilOpState
{
	VkStencilOp fail_op{VK_STENCIL_OP_REPLACE};

	VkStencilOp pass_op{VK_STENCIL_OP_REPLACE};

	VkStencilOp depth_fail_op{VK_STENCIL_OP_REPLACE};

	VkCompareOp compare_op{VK_COMPARE_OP_NEVER};
};

struct DepthStencilState
{
	VkBool32 depth_test_enable{VK_TRUE};

	VkBool32 depth_write_enable{VK_TRUE};

	VkCompareOp depth_compare_op{VK_COMPARE_OP_LESS_OR_EQUAL};

	VkBool32 depth_bounds_test_enable{VK_FALSE};

	VkBool32 stencil_test_enable{VK_FALSE};

	StencilOpState front{};

	StencilOpState back{};
};

struct ColorBlendAttachmentState
{
	VkBool32 blend_enable{VK_FALSE};

	VkBlendFactor src_color_blend_factor{VK_BLEND_FACTOR_ONE};

	VkBlendFactor dst_color_blend_factor{VK_BLEND_FACTOR_ZERO};

	VkBlendOp color_blend_op{VK_BLEND_OP_ADD};

	VkBlendFactor src_alpha_blend_factor{VK_BLEND_FACTOR_ONE};

	VkBlendFactor dst_alpha_blend_factor{VK_BLEND_FACTOR_ZERO};

	VkBlendOp alpha_blend_op{VK_BLEND_OP_ADD};

	VkColorComponentFlags color_write_mask{VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
};

struct ColorBlendState
{
	VkBool32 logic_op_enable{VK_FALSE};

	VkLogicOp logic_op{VK_LOGIC_OP_CLEAR};

	std::vector<ColorBlendAttachmentState> attachments;
};

/// Helper class to create specialization constants for a vulkan pipeline
class SpecializationInfo
{
  public:
	const VkSpecializationInfo &get_handle() const;

	template <class T>
	void set_constant(uint32_t constant_id, const T &data);

	void set_constant(uint32_t constant_id, const std::vector<uint8_t> &data);

	const std::vector<uint8_t> &get_data() const;

	const std::vector<VkSpecializationMapEntry> &get_map_entries() const;

  private:
	mutable VkSpecializationInfo handle;

	std::vector<VkSpecializationMapEntry> map_entries;

	std::vector<uint8_t> data;
};

template <class T>
inline void SpecializationInfo::set_constant(std::uint32_t constant_id, const T &data)
{
	std::uint32_t value = static_cast<std::uint32_t>(data);

	set_constant(constant_id,
	             {reinterpret_cast<const uint8_t *>(&value),
	              reinterpret_cast<const uint8_t *>(&value) + sizeof(T)});
}

template <>
inline void SpecializationInfo::set_constant<bool>(std::uint32_t constant_id, const bool &data)
{
	std::uint32_t value = static_cast<std::uint32_t>(data);

	set_constant(
	    constant_id,
	    {reinterpret_cast<const uint8_t *>(&value),
	     reinterpret_cast<const uint8_t *>(&value) + sizeof(std::uint32_t)});
}

class GraphicsPipelineState
{
  public:
	void reset();

	void set_pipeline_layout(PipelineLayout &pipeline_layout);

	void set_render_pass(const RenderPass &render_pass);

	void set_vertex_input_state(const VertexInputState &vertex_input_sate);

	void set_input_assembly_state(const InputAssemblyState &input_assembly_state);

	void set_rasterization_state(const RasterizationState &rasterization_state);

	void set_viewport_state(const ViewportState &viewport_state);

	void set_multisample_state(const MultisampleState &multisample_state);

	void set_depth_stencil_state(const DepthStencilState &depth_stencil_state);

	void set_color_blend_state(const ColorBlendState &color_blend_state);

	void set_subpass_index(uint32_t subpass_index);

	const PipelineLayout &get_pipeline_layout() const;

	const RenderPass &get_render_pass() const;

	const VertexInputState &get_vertex_input_state() const;

	const InputAssemblyState &get_input_assembly_state() const;

	const RasterizationState &get_rasterization_state() const;

	const ViewportState &get_viewport_state() const;

	const MultisampleState &get_multisample_state() const;

	const DepthStencilState &get_depth_stencil_state() const;

	const ColorBlendState &get_color_blend_state() const;

	uint32_t get_subpass_index() const;

	bool is_dirty() const;

	void clear_dirty();

  private:
	bool dirty{false};

	PipelineLayout *pipeline_layout{nullptr};

	const RenderPass *render_pass{nullptr};

	VertexInputState vertex_input_sate{};

	InputAssemblyState input_assembly_state{};

	RasterizationState rasterization_state{};

	ViewportState viewport_state{};

	MultisampleState multisample_state{};

	DepthStencilState depth_stencil_state{};

	ColorBlendState color_blend_state{};

	uint32_t subpass_index{0U};
};
}        // namespace vkb
