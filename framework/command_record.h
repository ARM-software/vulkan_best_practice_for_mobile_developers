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

#include <list>

#include "common/vk_common.h"
#include "core/descriptor_set.h"
#include "core/framebuffer.h"
#include "core/image_view.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/render_pass.h"
#include "core/sampler.h"
#include "rendering/pipeline_state.h"
#include "rendering/render_target.h"
#include "resource_binding_state.h"

namespace vkb
{
class RenderContext;
class CommandBuffer;

/*
 * @brief Temporary pipeline descriptor structure for a drawcall 
 *        in a subpass to be used during command recording.
 */
struct PipelineDesc
{
	std::streampos event_id{};

	PipelineState pipeline_state;
};

/*
 * @brief Temporary subpass descriptor structure for a renderpass to
 *        to be used during command recording.
 */
struct SubpassDesc
{
	std::streampos event_id{};

	std::vector<uint32_t> input_attachments;

	std::vector<uint32_t> output_attachments;

	std::list<PipelineDesc> pipeline_descs;
};

/*
 * @brief Render pass binding to be used during command replay.
 */
struct RenderPassBinding
{
	std::streampos event_id;

	const RenderTarget &render_target;

	std::vector<LoadStoreInfo> load_store_infos;

	std::vector<VkClearValue> clear_values;

	std::vector<SubpassDesc> subpasses;

	const RenderPass *render_pass;

	const Framebuffer *framebuffer;

	VkSubpassContents contents{VK_SUBPASS_CONTENTS_INLINE};
};

/*
 * @brief Pipeline binding structure to be used during command replay.
 */
struct PipelineBinding
{
	std::streampos event_id;

	VkPipelineBindPoint pipeline_bind_point;

	const Pipeline &pipeline;
};

/*
 * @brief Descriptor set binding structure to be used during command replay.
 */
struct DescriptorSetBinding
{
	std::streampos event_id;

	VkPipelineBindPoint pipeline_bind_point;

	const PipelineLayout &pipeline_layout;

	uint32_t set_index;

	const DescriptorSet &descriptor_set;

	std::vector<uint32_t> dynamic_offsets;
};

/**
 * @brief Command Type Enum
 * 
 */
enum class CommandType
{
	Begin,
	End,
	BeginRenderPass,
	NextSubpass,
	EndRenderPass,
	BindPipelineLayout,
	ExecuteCommands,
	PushConstants,
	BindBuffer,
	BindImage,
	BindVertexBuffers,
	BindIndexBuffer,
	SetVertexInputFormat,
	SetViewportState,
	SetInputAssemblyState,
	SetRasterizationState,
	SetMultisampleState,
	SetDepthStencilState,
	SetColorBlendState,
	SetViewport,
	SetScissor,
	SetLineWidth,
	SetDepthBias,
	SetBlendConstants,
	SetDepthBounds,
	Draw,
	DrawIndexed,
	DrawIndexedIndirect,
	Dispatch,
	DispatchIndirect,
	UpdateBuffer,
	BlitImage,
	CopyImage,
	CopyBufferToImage,
	ImageMemoryBarrier,
	BufferMemoryBarrier
};

/*
 * @brief Writes Vulkan commands in a memory stream while building 
 *        Vulkan pipelines and descriptor sets for each draw only if state changes.
 */
class CommandRecord
{
  public:
	CommandRecord(Device &device);

	void reset();

	Device &get_device();

	const std::ostringstream &get_stream() const;

	std::vector<RenderPassBinding> &get_render_pass_bindings();

	const std::vector<PipelineBinding> &get_pipeline_bindings() const;

	const std::vector<DescriptorSetBinding> &get_descriptor_set_bindings() const;

	void begin(VkCommandBufferUsageFlags flags);

	void end();

	void begin_render_pass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values, VkSubpassContents contents);

	void next_subpass();

	void resolve_subpasses();

	void execute_commands(std::vector<vkb::CommandBuffer *> &sec_cmd_bufs);

	void end_render_pass();

	void bind_pipeline_layout(PipelineLayout &pipeline_layout);

	void set_specialization_constant(uint32_t constant_id, const std::vector<uint8_t> &value);

	void push_constants(uint32_t offset, const std::vector<uint8_t> &values);

	/**
	 * @brief It binds a buffer to an uniform
	 * @param buffer Buffer to bind
	 * @param offset Offset to apply to the buffer
	 * @param range Memory size to bind
	 * @param set Destination descriptor set
	 * @param binding Descriptor binding within that set
	 * @param array_element The starting element in that array
	 */
	void bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);

	/**
	 * @brief It binds an image view to an uniform sampler
	 * @param image_view Image view to bind
	 * @param sampler Samper to use
	 * @param set Destination descriptor set
	 * @param binding Descriptor binding within that set
	 * @param array_element The starting element in that array
	 */
	void bind_image(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element);

	/**
	 * @brief Like bind_image, it binds an image view to an input attachment
	 * @param image_view Image view to bind
	 * @param set Destination descriptor set
	 * @param binding Descriptor binding within that set
	 * @param array_element The starting element in that array
	 */
	void bind_input(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_vertex_buffers(uint32_t first_binding, const std::vector<std::reference_wrapper<const vkb::core::Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets);

	void bind_index_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type);

	/**
	 * @brief Set the viewport state object
	 * 
	 * @param state_info 
	 */
	void set_viewport_state(const ViewportState &state_info);

	/**
	 * @brief Set the vertex input state object
	 * 
	 * @param state_info 
	 */
	void set_vertex_input_state(const VertexInputState &state_info);

	/**
	 * @brief Set the input assembly state object
	 * 
	 * @param state_info 
	 */
	void set_input_assembly_state(const InputAssemblyState &state_info);

	/**
	 * @brief Set the rasterization state object
	 * 
	 * @param state_info 
	 */
	void set_rasterization_state(const RasterizationState &state_info);

	/**
	 * @brief Set the multisample state object
	 * 
	 * @param state_info 
	 */
	void set_multisample_state(const MultisampleState &state_info);

	/**
	 * @brief Set the depth stencil state object
	 * 
	 * @param state_info 
	 */
	void set_depth_stencil_state(const DepthStencilState &state_info);

	/**
	 * @brief Set the color blend state object
	 * 
	 * @param state_info 
	 */
	void set_color_blend_state(const ColorBlendState &state_info);

	/**
	 * @brief Set the viewport object
	 * 
	 * @param first_viewport 
	 * @param viewports 
	 */
	void set_viewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports);

	/**
	 * @brief Set the scissor object
	 * 
	 * @param first_scissor 
	 * @param scissors 
	 */
	void set_scissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors);

	/**
	 * @brief Set the line width object
	 * 
	 * @param line_width 
	 */
	void set_line_width(float line_width);

	/**
	 * @brief Set the depth bias object
	 * 
	 * @param depth_bias_constant_factor 
	 * @param depth_bias_clamp 
	 * @param depth_bias_slope_factor 
	 */
	void set_depth_bias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor);

	/**
	 * @brief Set the blend constants object
	 * 
	 * @param blend_constants 
	 */
	void set_blend_constants(const std::array<float, 4> &blend_constants);

	/**
	 * @brief Set the depth bounds object
	 * 
	 * @param min_depth_bounds 
	 * @param max_depth_bounds 
	 */
	void set_depth_bounds(float min_depth_bounds, float max_depth_bounds);

	void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

	void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);

	void draw_indexed_indirect(const core::Buffer &buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride);

	void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

	void dispatch_indirect(const core::Buffer &buffer, VkDeviceSize offset);

	void update_buffer(const core::Buffer &buffer, VkDeviceSize offset, const std::vector<uint8_t> &data);

	void blit_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageBlit> &regions);

	void copy_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageCopy> &regions);

	void copy_buffer_to_image(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions);

	void image_memory_barrier(const core::ImageView &image_view, const ImageMemoryBarrier &memory_barrier);

	void buffer_memory_barrier(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize size, const BufferMemoryBarrier &memory_barrier);

  private:
	Device &device;

	std::ostringstream stream;

	std::vector<RenderPassBinding> render_pass_bindings;

	std::vector<DescriptorSetBinding> descriptor_set_bindings;

	std::vector<PipelineBinding> pipeline_bindings;

	PipelineState pipeline_state;

	ResourceBindingState resource_binding_state;

	std::unordered_map<uint32_t, DescriptorSetLayout *> descriptor_set_layout_state;

	void prepare_pipeline_bindings(CommandRecord &recorder, RenderPassBinding &render_pass_desc);

	/**
	 * @brief Flush the piplines state
	 * 
	 */
	void flush_pipeline_state(VkPipelineBindPoint pipeline_bind_point);

	/**
	 * @brief Flush the descriptor set State
	 * 
	 */
	void flush_descriptor_state(VkPipelineBindPoint pipeline_bind_point);
};
}        // namespace vkb
