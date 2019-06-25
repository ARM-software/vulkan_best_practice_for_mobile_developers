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

namespace vkb
{
class CommandBuffer;

/*
 * @brief Reads Vulkan commands from a memory stream and runs them in a Vulkan command buffer.
 */
class CommandReplay
{
  public:
	CommandReplay();

	/*
	 * @brief Reads Vulkan commands from a CommandRecord object and calls the 
	 *        corresponding Vulkan function to record the command in a Vulkan command buffer object.
	 */
	void play(CommandBuffer &command_buffer, CommandRecord &recorder);

  protected:
	using CommandFunc = std::function<void(CommandBuffer &, std::istringstream &)>;

	std::unordered_map<CommandType, CommandFunc> stream_commands;

  private:
	void begin(CommandBuffer &command_buffer, std::istringstream &stream);

	void end(CommandBuffer &command_buffer, std::istringstream &stream);

	void next_subpass(CommandBuffer &command_buffer, std::istringstream &stream);

	void end_render_pass(CommandBuffer &command_buffer, std::istringstream &stream);

	void push_constants(CommandBuffer &command_buffer, std::istringstream &stream);

	void bind_vertex_buffers(CommandBuffer &command_buffer, std::istringstream &stream);

	void bind_index_buffer(CommandBuffer &command_buffer, std::istringstream &stream);

	void set_viewport(CommandBuffer &command_buffer, std::istringstream &stream);

	void set_scissor(CommandBuffer &command_buffer, std::istringstream &stream);

	void set_line_width(CommandBuffer &command_buffer, std::istringstream &stream);

	void set_depth_bias(CommandBuffer &command_buffer, std::istringstream &stream);

	void set_blend_constants(CommandBuffer &command_buffer, std::istringstream &stream);

	void set_depth_bounds(CommandBuffer &command_buffer, std::istringstream &stream);

	void draw(CommandBuffer &command_buffer, std::istringstream &stream);

	void draw_indexed(CommandBuffer &command_buffer, std::istringstream &stream);

	void update_buffer(CommandBuffer &command_buffer, std::istringstream &stream);

	void blit_image(CommandBuffer &command_buffer, std::istringstream &stream);

	void copy_image(CommandBuffer &command_buffer, std::istringstream &stream);

	void copy_buffer_to_image(CommandBuffer &command_buffer, std::istringstream &stream);

	void image_memory_barrier(CommandBuffer &command_buffer, std::istringstream &stream);
};
}        // namespace vkb
