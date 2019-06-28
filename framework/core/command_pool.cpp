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

#include "command_pool.h"

#include "device.h"

namespace vkb
{
CommandPool::CommandPool(Device &d, uint32_t queue_family_index) :
    device{d}
{
	VkCommandPoolCreateInfo create_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};

	create_info.queueFamilyIndex = queue_family_index;
	create_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	auto result = vkCreateCommandPool(device.get_handle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Failed to create command pool"};
	}
}

CommandPool::~CommandPool()
{
	command_buffers.clear();

	// Destroy command pool
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(device.get_handle(), handle, nullptr);
	}
}

CommandPool::CommandPool(CommandPool &&other) :
    device{other.device},
    handle{other.handle},
    queue_family_index{other.queue_family_index},
    command_buffers{std::move(other.command_buffers)},
    active_command_buffer_count{other.active_command_buffer_count}
{
	other.handle = VK_NULL_HANDLE;

	other.queue_family_index = 0;

	other.active_command_buffer_count = 0;
}

Device &CommandPool::get_device()
{
	return device;
}

uint32_t CommandPool::get_queue_family_index() const
{
	return queue_family_index;
}

VkCommandPool CommandPool::get_handle() const
{
	return handle;
}

VkResult CommandPool::reset()
{
	VkResult result = vkResetCommandPool(device.get_handle(), handle, 0);

	if (result != VK_SUCCESS)
	{
		return result;
	}

	active_command_buffer_count = 0;

	return VK_SUCCESS;
}

CommandBuffer &CommandPool::request_command_buffer(VkCommandBufferLevel level)
{
	if (active_command_buffer_count < command_buffers.size())
	{
		return *command_buffers.at(active_command_buffer_count++);
	}

	command_buffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));

	active_command_buffer_count++;

	return *command_buffers.back();
}
}        // namespace vkb
