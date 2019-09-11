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

#include "common/helpers.h"
#include "common/vk_common.h"
#include "core/command_buffer.h"

namespace vkb
{
class Device;
class RenderFrame;

class CommandPool
{
  public:
	CommandPool(Device &device, uint32_t queue_family_index, RenderFrame *render_frame = nullptr,
	            CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool);

	CommandPool(const CommandPool &) = delete;

	CommandPool(CommandPool &&other);

	~CommandPool();

	CommandPool &operator=(const CommandPool &) = delete;

	CommandPool &operator=(CommandPool &&) = delete;

	Device &get_device();

	uint32_t get_queue_family_index() const;

	VkCommandPool get_handle() const;

	RenderFrame *get_render_frame();

	VkResult reset_pool();

	CommandBuffer &request_command_buffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	const CommandBuffer::ResetMode get_reset_mode() const;

  private:
	Device &device;

	VkCommandPool handle{VK_NULL_HANDLE};

	RenderFrame *render_frame{nullptr};

	uint32_t queue_family_index{0};

	std::vector<std::unique_ptr<CommandBuffer>> command_buffers;

	uint32_t active_command_buffer_count{0};

	CommandBuffer::ResetMode reset_mode{CommandBuffer::ResetMode::ResetPool};

	VkResult reset_command_buffers();
};
}        // namespace vkb
