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

#include "command_buffer.h"

#include "command_pool.h"
#include "device.h"

namespace vkb
{
CommandBuffer::CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level) :
    command_pool{command_pool},
    recorder{command_pool.get_device()}
{
	VkCommandBufferAllocateInfo allocate_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};

	allocate_info.commandPool        = command_pool.get_handle();
	allocate_info.commandBufferCount = 1;
	allocate_info.level              = level;

	VkResult result = vkAllocateCommandBuffers(command_pool.get_device().get_handle(), &allocate_info, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Failed to allocate command buffer"};
	}
}

CommandBuffer::~CommandBuffer()
{
	// Destroy command buffer
	if (handle != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(command_pool.get_device().get_handle(), command_pool.get_handle(), 1, &handle);
	}
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) :
    command_pool{other.command_pool},
    handle{other.handle},
    recorder{std::move(other.recorder)},
    replayer{std::move(other.replayer)}
{
	other.handle = VK_NULL_HANDLE;
}

Device &CommandBuffer::get_device()
{
	return command_pool.get_device();
}

CommandRecord &CommandBuffer::get_recorder()
{
	return recorder;
}

const VkCommandBuffer &CommandBuffer::get_handle() const
{
	return handle;
}

VkResult CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
	assert(!recording_commands && "Command buffer is already recording, please call end before beginning again");

	if (recording_commands)
	{
		return VK_NOT_READY;
	}

	recorder.reset();

	recording_commands = true;

	recorder.begin(flags);

	return VK_SUCCESS;
}

VkResult CommandBuffer::end()
{
	assert(recording_commands && "Command buffer is not recording, please call begin before end");

	if (!recording_commands)
	{
		return VK_NOT_READY;
	}

	recording_commands = false;

	recorder.end();

	replayer.play(*this, recorder);

	return VK_SUCCESS;
}

void CommandBuffer::begin_render_pass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values)
{
	recorder.begin_render_pass(render_target, load_store_infos, clear_values);
}

void CommandBuffer::next_subpass()
{
	recorder.next_subpass();
}

void CommandBuffer::end_render_pass()
{
	recorder.end_render_pass();
}

void CommandBuffer::bind_pipeline_layout(PipelineLayout &pipeline_layout)
{
	recorder.bind_pipeline_layout(pipeline_layout);
}

void CommandBuffer::push_constants(uint32_t offset, const std::vector<uint8_t> &values)
{
	recorder.push_constants(offset, values);
}

void CommandBuffer::bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element)
{
	recorder.bind_buffer(buffer, offset, range, set, binding, array_element);
}

void CommandBuffer::bind_image(const ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element)
{
	recorder.bind_image(image_view, sampler, set, binding, array_element);
}

void CommandBuffer::bind_vertex_buffers(uint32_t first_binding, const std::vector<std::reference_wrapper<const vkb::core::Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets)
{
	recorder.bind_vertex_buffers(first_binding, buffers, offsets);
}

void CommandBuffer::bind_index_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type)
{
	recorder.bind_index_buffer(buffer, offset, index_type);
}

void CommandBuffer::set_viewport_state(const ViewportState &state_info)
{
	recorder.set_viewport_state(state_info);
}

void CommandBuffer::set_vertex_input_state(const VertexInputState &state_info)
{
	recorder.set_vertex_input_state(state_info);
}

void CommandBuffer::set_input_assembly_state(const InputAssemblyState &state_info)
{
	recorder.set_input_assembly_state(state_info);
}

void CommandBuffer::set_rasterization_state(const RasterizationState &state_info)
{
	recorder.set_rasterization_state(state_info);
}

void CommandBuffer::set_multisample_state(const MultisampleState &state_info)
{
	recorder.set_multisample_state(state_info);
}

void CommandBuffer::set_depth_stencil_state(const DepthStencilState &state_info)
{
	recorder.set_depth_stencil_state(state_info);
}

void CommandBuffer::set_color_blend_state(const ColorBlendState &state_info)
{
	recorder.set_color_blend_state(state_info);
}

void CommandBuffer::set_viewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports)
{
	recorder.set_viewport(first_viewport, viewports);
}

void CommandBuffer::set_scissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors)
{
	recorder.set_scissor(first_scissor, scissors);
}

void CommandBuffer::set_line_width(float line_width)
{
	recorder.set_line_width(line_width);
}

void CommandBuffer::set_depth_bias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor)
{
	recorder.set_depth_bias(depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor);
}

void CommandBuffer::set_blend_constants(const std::array<float, 4> &blend_constants)
{
	recorder.set_blend_constants(blend_constants);
}

void CommandBuffer::set_depth_bounds(float min_depth_bounds, float max_depth_bounds)
{
	recorder.set_depth_bounds(min_depth_bounds, max_depth_bounds);
}

void CommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
	recorder.draw(vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{
	recorder.draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandBuffer::update_buffer(const core::Buffer &buffer, VkDeviceSize offset, const std::vector<uint8_t> &data)
{
	recorder.update_buffer(buffer, offset, data);
}

void CommandBuffer::copy_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageCopy> &regions)
{
	recorder.copy_image(src_img, dst_img, regions);
}

void CommandBuffer::copy_buffer_to_image(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions)
{
	recorder.copy_buffer_to_image(buffer, image, regions);
}

void CommandBuffer::image_memory_barrier(const ImageView &image_view, const ImageMemoryBarrier &memory_barriers)
{
	recorder.image_memory_barrier(image_view, memory_barriers);
}
}        // namespace vkb
