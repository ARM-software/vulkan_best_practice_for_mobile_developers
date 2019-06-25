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

#include "command_record.h"
#include "command_replay.h"
#include "core/buffer.h"
#include "core/image.h"
#include "core/image_view.h"
#include "core/sampler.h"
#include "rendering/graphics_pipeline_state.h"
#include "rendering/render_target.h"

namespace vkb
{
class CommandPool;

/**
 * @brief Records Vulkan commands after begin function and replays them before end function is called.
 *        Helper class to build graphics/compute pipelines and descriptor sets
 */
class CommandBuffer : public NonCopyable
{
  public:
	CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level);

	~CommandBuffer();

	/**
	 * @brief Move constructs
	 */
	CommandBuffer(CommandBuffer &&other);

	Device &get_device();

	CommandRecord &get_recorder();

	const VkCommandBuffer &get_handle() const;

	bool is_recording() const
	{
		return recording_commands;
	}

	VkResult begin(VkCommandBufferUsageFlags flags);

	VkResult end();

	void begin_render_pass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values);

	void next_subpass();

	void end_render_pass();

	void bind_pipeline_layout(PipelineLayout &pipeline_layout);

	void push_constants(uint32_t offset, const std::vector<uint8_t> &values);

	template <typename T>
	void push_constants(uint32_t offset, const T &value)
	{
		push_constants(offset,
		               std::vector<uint8_t>{reinterpret_cast<const uint8_t *>(&value),
		                                    reinterpret_cast<const uint8_t *>(&value) + sizeof(T)});
	}

	void bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_image(const ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_input(const ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_vertex_buffers(uint32_t first_binding, const std::vector<std::reference_wrapper<const vkb::core::Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets);

	void bind_index_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type);

	void set_viewport_state(const ViewportState &state_info);

	void set_vertex_input_state(const VertexInputState &state_info);

	void set_input_assembly_state(const InputAssemblyState &state_info);

	void set_rasterization_state(const RasterizationState &state_info);

	void set_multisample_state(const MultisampleState &state_info);

	void set_depth_stencil_state(const DepthStencilState &state_info);

	void set_color_blend_state(const ColorBlendState &state_info);

	void set_viewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports);

	void set_scissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors);

	void set_line_width(float line_width);

	void set_depth_bias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor);

	void set_blend_constants(const std::array<float, 4> &blend_constants);

	void set_depth_bounds(float min_depth_bounds, float max_depth_bounds);

	void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

	void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);

	void update_buffer(const core::Buffer &buffer, VkDeviceSize offset, const std::vector<uint8_t> &data);

	void blit_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageBlit> &regions);

	void copy_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageCopy> &regions);

	void copy_buffer_to_image(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions);

	void image_memory_barrier(const ImageView &image_view, const ImageMemoryBarrier &memory_barrier);

  private:
	bool recording_commands{false};

	CommandPool &command_pool;

	VkCommandBuffer handle{VK_NULL_HANDLE};

	CommandRecord recorder;

	CommandReplay replayer;
};
}        // namespace vkb
