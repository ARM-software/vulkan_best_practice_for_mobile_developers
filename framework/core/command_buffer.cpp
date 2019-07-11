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
#include "common/error.h"
#include "device.h"

namespace vkb
{
CommandBuffer::CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level) :
    command_pool{command_pool},
    level{level},
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
    usage_flags{other.usage_flags},
    level{other.level},
    handle{other.handle},
    recorder{std::move(other.recorder)},
    replayer{std::move(other.replayer)},
    state{other.state}
{
	other.handle = VK_NULL_HANDLE;
	other.state  = State::Invalid;
}

Device &CommandBuffer::get_device()
{
	return command_pool.get_device();
}

CommandRecord &CommandBuffer::get_recorder()
{
	return recorder;
}

CommandReplay &CommandBuffer::get_replayer()
{
	return replayer;
}

const VkCommandBuffer &CommandBuffer::get_handle() const
{
	return handle;
}

bool CommandBuffer::is_recording() const
{
	return state == State::Recording;
}

VkResult CommandBuffer::begin(VkCommandBufferUsageFlags flags, CommandBuffer *primary_cmd_buf)
{
	assert(!is_recording() && "Command buffer is already recording, please call end before beginning again");

	if (is_recording())
	{
		return VK_NOT_READY;
	}

	recorder.reset();

	state = State::Recording;

	usage_flags = flags;

	if (level != VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		recorder.begin(flags);
	}
	else
	{
		// Secondary command buffers' Vulkan begin command is deferred further, when the information required
		// to set up inheritance information is known
		assert(primary_cmd_buf && "A primary command buffer pointer must be provided when calling begin from a secondary one");

		recorder.get_render_pass_bindings().push_back(primary_cmd_buf->get_recorder().get_render_pass_bindings().back());
	}

	return VK_SUCCESS;
}

VkResult CommandBuffer::end()
{
	assert(is_recording() && "Command buffer is not recording, please call begin before end");

	if (!is_recording())
	{
		return VK_NOT_READY;
	}

	recorder.end();

	if (level != VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		// Secondary buffers play is deferred further
		replayer.play(*this, recorder);
	}

	state = State::Executable;

	return VK_SUCCESS;
}

void CommandBuffer::begin_render_pass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values, VkSubpassContents contents)
{
	recorder.begin_render_pass(render_target, load_store_infos, clear_values, contents);
}

void CommandBuffer::next_subpass()
{
	recorder.next_subpass();
}

void CommandBuffer::resolve_subpasses()
{
	recorder.resolve_subpasses();
}

void CommandBuffer::execute_commands(std::vector<CommandBuffer *> &secondary_command_buffers)
{
	recorder.execute_commands(secondary_command_buffers);
}

void CommandBuffer::end_render_pass()
{
	recorder.end_render_pass();
}

void CommandBuffer::bind_pipeline_layout(PipelineLayout &pipeline_layout)
{
	recorder.bind_pipeline_layout(pipeline_layout);
}

void CommandBuffer::set_specialization_constant(uint32_t constant_id, const std::vector<uint8_t> &data)
{
	recorder.set_specialization_constant(constant_id, data);
}

void CommandBuffer::push_constants(uint32_t offset, const std::vector<uint8_t> &values)
{
	recorder.push_constants(offset, values);
}

void CommandBuffer::bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element)
{
	recorder.bind_buffer(buffer, offset, range, set, binding, array_element);
}

void CommandBuffer::bind_image(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element)
{
	recorder.bind_image(image_view, sampler, set, binding, array_element);
}

void CommandBuffer::bind_input(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element)
{
	recorder.bind_input(image_view, set, binding, array_element);
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

void CommandBuffer::draw_indexed_indirect(const core::Buffer &buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride)
{
	recorder.draw_indexed_indirect(buffer, offset, draw_count, stride);
}

void CommandBuffer::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
	recorder.dispatch(group_count_x, group_count_y, group_count_z);
}

void CommandBuffer::dispatch_indirect(const core::Buffer &buffer, VkDeviceSize offset)
{
	recorder.dispatch_indirect(buffer, offset);
}

void CommandBuffer::update_buffer(const core::Buffer &buffer, VkDeviceSize offset, const std::vector<uint8_t> &data)
{
	recorder.update_buffer(buffer, offset, data);
}

void CommandBuffer::blit_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageBlit> &regions)
{
	recorder.blit_image(src_img, dst_img, regions);
}

void CommandBuffer::copy_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageCopy> &regions)
{
	recorder.copy_image(src_img, dst_img, regions);
}

void CommandBuffer::copy_buffer_to_image(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions)
{
	recorder.copy_buffer_to_image(buffer, image, regions);
}

void CommandBuffer::image_memory_barrier(const core::ImageView &image_view, const ImageMemoryBarrier &memory_barriers)
{
	recorder.image_memory_barrier(image_view, memory_barriers);
}

void CommandBuffer::buffer_memory_barrier(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize size, const BufferMemoryBarrier &memory_barrier)
{
	recorder.buffer_memory_barrier(buffer, offset, size, memory_barrier);
}

const CommandBuffer::State CommandBuffer::get_state() const
{
	return state;
}

const VkCommandBufferUsageFlags CommandBuffer::get_usage_flags() const
{
	return usage_flags;
}

VkResult CommandBuffer::reset(ResetMode reset_mode)
{
	VkResult result = VK_SUCCESS;

	assert(reset_mode == command_pool.get_reset_mode() && "Command buffer reset mode must match the one used by the pool to allocate it");

	state = State::Initial;

	if (reset_mode == ResetMode::ResetIndividually)
	{
		result = vkResetCommandBuffer(handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	return result;
}
}        // namespace vkb
