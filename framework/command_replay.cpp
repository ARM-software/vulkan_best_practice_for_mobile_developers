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

#include "command_replay.h"

#include "core/command_buffer.h"
#include "core/descriptor_set.h"

namespace vkb
{
namespace
{
template <typename T>
inline void read(std::istringstream &is, T &value)
{
	is.read(reinterpret_cast<char *>(&value), sizeof(T));
}

template <class T>
inline void read(std::istringstream &is, std::vector<T> &value)
{
	std::size_t size;
	read(is, size);
	value.resize(size);
	is.read(reinterpret_cast<char *>(value.data()), value.size() * sizeof(T));
}

template <class T, uint32_t N>
inline void read(std::istringstream &is, std::array<T, N> &value)
{
	is.read(reinterpret_cast<char *>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void read(std::istringstream &is, T &first_arg, Args &... args)
{
	read(is, first_arg);

	read(is, args...);
}
}        // namespace

CommandReplay::CommandReplay()
{
	stream_commands[CommandType::Begin]              = std::bind(&CommandReplay::begin, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::End]                = std::bind(&CommandReplay::end, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::NextSubpass]        = std::bind(&CommandReplay::next_subpass, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::EndRenderPass]      = std::bind(&CommandReplay::end_render_pass, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::PushConstants]      = std::bind(&CommandReplay::push_constants, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::BindVertexBuffers]  = std::bind(&CommandReplay::bind_vertex_buffers, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::BindIndexBuffer]    = std::bind(&CommandReplay::bind_index_buffer, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::SetViewport]        = std::bind(&CommandReplay::set_viewport, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::SetScissor]         = std::bind(&CommandReplay::set_scissor, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::SetLineWidth]       = std::bind(&CommandReplay::set_line_width, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::SetDepthBias]       = std::bind(&CommandReplay::set_depth_bias, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::SetBlendConstants]  = std::bind(&CommandReplay::set_blend_constants, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::SetDepthBounds]     = std::bind(&CommandReplay::set_depth_bounds, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::Draw]               = std::bind(&CommandReplay::draw, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::DrawIndexed]        = std::bind(&CommandReplay::draw_indexed, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::UpdateBuffer]       = std::bind(&CommandReplay::update_buffer, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::CopyImage]          = std::bind(&CommandReplay::copy_image, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::CopyBufferToImage]  = std::bind(&CommandReplay::copy_buffer_to_image, this, std::placeholders::_1, std::placeholders::_2);
	stream_commands[CommandType::ImageMemoryBarrier] = std::bind(&CommandReplay::image_memory_barrier, this, std::placeholders::_1, std::placeholders::_2);
}

void CommandReplay::play(CommandBuffer &command_buffer, CommandRecord &recorder)
{
	std::istringstream stream{recorder.get_stream().str()};

	// Get the first render pass to bind
	auto render_pass_binding_it = recorder.get_render_pass_bindings().cbegin();

	// Get the first pipeline bindings to bind.
	auto pipeline_binding_it = recorder.get_pipeline_bindings().cbegin();

	// Get the first descriptor set to bind
	auto descriptor_set_binding_it = recorder.get_descriptor_set_bindings().cbegin();

	while (true)
	{
		// Get current event id
		std::streampos event_id = stream.tellg();

		// Check to see if there are any render passes left
		if (render_pass_binding_it != recorder.get_render_pass_bindings().cend())
		{
			// Current render pass event id must be equal to the current event id
			if (render_pass_binding_it->event_id == event_id)
			{
				VkRenderPassBeginInfo begin_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

				begin_info.renderPass        = render_pass_binding_it->render_pass->get_handle();
				begin_info.framebuffer       = render_pass_binding_it->framebuffer->get_handle();
				begin_info.renderArea.extent = render_pass_binding_it->render_target.get_extent();
				begin_info.clearValueCount   = to_u32(render_pass_binding_it->clear_values.size());
				begin_info.pClearValues      = render_pass_binding_it->clear_values.data();

				// Begin render pass
				vkCmdBeginRenderPass(command_buffer.get_handle(), &begin_info, VK_SUBPASS_CONTENTS_INLINE);

				// Move to the next render pass
				++render_pass_binding_it;
			}
		}

		// Check to see if there are any pipeline bindings left
		if (pipeline_binding_it != recorder.get_pipeline_bindings().cend())
		{
			// The next pipeline binding's event id must be equal to the current read position.
			if (pipeline_binding_it->event_id == event_id)
			{
				// Bind pipeline.
				vkCmdBindPipeline(command_buffer.get_handle(),
				                  pipeline_binding_it->pipeline_bind_point,
				                  pipeline_binding_it->pipeline.get_handle());

				// Move to the next pipeline binding
				++pipeline_binding_it;
			}
		}

		// Check to see if there are any descriptor set bindings left
		if (descriptor_set_binding_it != recorder.get_descriptor_set_bindings().cend())
		{
			// The next descriptor set binding's event id must be equal to the current read position.
			while (descriptor_set_binding_it->event_id == event_id)
			{
				VkDescriptorSet descriptor_set = descriptor_set_binding_it->descriptor_set.get_handle();

				// Bind descriptor set
				vkCmdBindDescriptorSets(command_buffer.get_handle(),
				                        descriptor_set_binding_it->pipeline_bind_point,
				                        descriptor_set_binding_it->pipeline_layout.get_handle(),
				                        descriptor_set_binding_it->set_index,
				                        1, &descriptor_set,
				                        descriptor_set_binding_it->dynamic_offsets.size(),
				                        descriptor_set_binding_it->dynamic_offsets.data());

				// Move to the next descriptor set binding
				if (++descriptor_set_binding_it == recorder.get_descriptor_set_bindings().cend())
				{
					break;
				}
			}
		}

		// Read command id
		CommandType cmd;
		read(stream, cmd);

		if (stream.eof())
		{
			break;
		}

		// Find command function for the given command id
		auto cmd_it = stream_commands.find(cmd);

		// Check if command replayer supports the given command
		if (cmd_it != stream_commands.end())
		{
			// Run command function
			stream_commands.at(cmd)(command_buffer, stream);
		}
		else
		{
			LOGE("Replay command not supported.");
		}
	}
}

void CommandReplay::begin(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

	// Read command parameters
	read(stream, begin_info.flags);

	// Call Vulkan function
	vkBeginCommandBuffer(command_buffer.get_handle(), &begin_info);
}

void CommandReplay::end(CommandBuffer &command_buffer, std::istringstream &stream)
{
	// Call Vulkan function
	vkEndCommandBuffer(command_buffer.get_handle());
}

void CommandReplay::next_subpass(CommandBuffer &command_buffer, std::istringstream &stream)
{
	// Call Vulkan function
	vkCmdNextSubpass(command_buffer.get_handle(), VK_SUBPASS_CONTENTS_INLINE);
}

void CommandReplay::end_render_pass(CommandBuffer &command_buffer, std::istringstream &stream)
{
	// Call Vulkan function
	vkCmdEndRenderPass(command_buffer.get_handle());
}

void CommandReplay::push_constants(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkPipelineLayout     pipeline_layout;
	VkShaderStageFlags   shader_stage;
	uint32_t             offset;
	std::vector<uint8_t> values;

	// Read command parameters
	read(stream, pipeline_layout, shader_stage, offset, values);

	// Call Vulkan function
	vkCmdPushConstants(command_buffer.get_handle(), pipeline_layout, shader_stage, offset, values.size(), values.data());
}

void CommandReplay::bind_vertex_buffers(CommandBuffer &command_buffer, std::istringstream &stream)
{
	uint32_t                  first_binding;
	std::vector<VkBuffer>     buffers;
	std::vector<VkDeviceSize> offsets;

	// Read command parameters
	read(stream, first_binding, buffers, offsets);

	// Call Vulkan function
	vkCmdBindVertexBuffers(command_buffer.get_handle(), first_binding, buffers.size(), buffers.data(), offsets.data());
}

void CommandReplay::bind_index_buffer(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkBuffer     buffer;
	VkDeviceSize offset;
	VkIndexType  index_type;

	// Read command parameters
	read(stream, buffer, offset, index_type);

	// Call Vulkan function
	vkCmdBindIndexBuffer(command_buffer.get_handle(), buffer, offset, index_type);
}

void CommandReplay::set_viewport(CommandBuffer &command_buffer, std::istringstream &stream)
{
	uint32_t                first_viewport;
	std::vector<VkViewport> viewports;

	// Read command parameters
	read(stream, first_viewport, viewports);

	// Call Vulkan function
	vkCmdSetViewport(command_buffer.get_handle(), first_viewport, viewports.size(), viewports.data());
}

void CommandReplay::set_scissor(CommandBuffer &command_buffer, std::istringstream &stream)
{
	uint32_t              first_scissor;
	std::vector<VkRect2D> scissors;

	// Read command parameters
	read(stream, first_scissor, scissors);

	// Call Vulkan function
	vkCmdSetScissor(command_buffer.get_handle(), first_scissor, scissors.size(), scissors.data());
}

void CommandReplay::set_line_width(CommandBuffer &command_buffer, std::istringstream &stream)
{
	float line_width;

	// Read command parameters
	read(stream, line_width);

	// Call Vulkan function
	vkCmdSetLineWidth(command_buffer.get_handle(), line_width);
}

void CommandReplay::set_depth_bias(CommandBuffer &command_buffer, std::istringstream &stream)
{
	float depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor;

	// Read command parameters
	read(stream, depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor);

	// Call Vulkan function
	vkCmdSetDepthBias(command_buffer.get_handle(), depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor);
}

void CommandReplay::set_blend_constants(CommandBuffer &command_buffer, std::istringstream &stream)
{
	std::array<float, 4> blend_constants;

	// Read command parameters
	read(stream, blend_constants);

	// Call Vulkan function
	vkCmdSetBlendConstants(command_buffer.get_handle(), blend_constants.data());
}

void CommandReplay::set_depth_bounds(CommandBuffer &command_buffer, std::istringstream &stream)
{
	float min_depth_bounds, max_depth_bounds;

	// Read command parameters
	read(stream, min_depth_bounds, max_depth_bounds);

	// Call Vulkan function
	vkCmdSetDepthBounds(command_buffer.get_handle(), min_depth_bounds, max_depth_bounds);
}

void CommandReplay::draw(CommandBuffer &command_buffer, std::istringstream &stream)
{
	uint32_t vertex_count;
	uint32_t instance_count;
	uint32_t first_vertex;
	uint32_t first_instance;

	// Read command parameters
	read(stream, vertex_count, instance_count, first_vertex, first_instance);

	// Call Vulkan function
	vkCmdDraw(command_buffer.get_handle(), vertex_count, instance_count, first_vertex, first_instance);
}

void CommandReplay::draw_indexed(CommandBuffer &command_buffer, std::istringstream &stream)
{
	uint32_t index_count;
	uint32_t instance_count;
	uint32_t first_index;
	uint32_t vertex_offset;
	uint32_t first_instance;

	// Read command parameters
	read(stream, index_count, instance_count, first_index, vertex_offset, first_instance);

	// Call Vulkan function
	vkCmdDrawIndexed(command_buffer.get_handle(), index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandReplay::update_buffer(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkBuffer             buffer;
	VkDeviceSize         offset;
	std::vector<uint8_t> data;

	// Read command parameters
	read(stream, buffer, offset, data);

	// Call Vulkan function
	vkCmdUpdateBuffer(command_buffer.get_handle(), buffer, offset, data.size(), data.data());
}

void CommandReplay::copy_image(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkImage                  src_image;
	VkImage                  dst_image;
	std::vector<VkImageCopy> regions;

	// Read command parameters
	read(stream, src_image, dst_image, regions);

	// Call Vulkan function
	vkCmdCopyImage(command_buffer.get_handle(), src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
}

void CommandReplay::copy_buffer_to_image(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkBuffer                       buffer;
	VkImage                        image;
	std::vector<VkBufferImageCopy> regions;

	// Read command parameters
	read(stream, buffer, image, regions);

	// Call Vulkan function
	vkCmdCopyBufferToImage(command_buffer.get_handle(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
}

void CommandReplay::image_memory_barrier(CommandBuffer &command_buffer, std::istringstream &stream)
{
	VkImage                 image;
	VkImageSubresourceRange subresource_range;
	ImageMemoryBarrier      memory_barrier;

	// Read command parameters
	read(stream, image, subresource_range, memory_barrier);

	VkImageMemoryBarrier image_memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

	image_memory_barrier.oldLayout        = memory_barrier.old_layout;
	image_memory_barrier.newLayout        = memory_barrier.new_layout;
	image_memory_barrier.image            = image;
	image_memory_barrier.subresourceRange = subresource_range;
	image_memory_barrier.srcAccessMask    = memory_barrier.src_access_mask;
	image_memory_barrier.dstAccessMask    = memory_barrier.dst_access_mask;

	VkPipelineStageFlags src_stage_mask = memory_barrier.src_stage_mask;
	VkPipelineStageFlags dst_stage_mask = memory_barrier.dst_stage_mask;

	// Call Vulkan function
	vkCmdPipelineBarrier(
	    command_buffer.get_handle(),
	    src_stage_mask,
	    dst_stage_mask,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1,
	    &image_memory_barrier);
}
}        // namespace vkb
