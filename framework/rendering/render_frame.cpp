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

#include "render_frame.h"

#include "common/logging.h"

namespace vkb
{
RenderFrame::RenderFrame(Device &device, RenderTarget &&render_target, uint16_t command_pool_count) :
    device{device},
    fence_pool{device},
    semaphore_pool{device},
    swapchain_render_target{std::move(render_target)},
    command_pool_count{command_pool_count}
{
	buffer_pools.emplace(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, std::make_pair(BufferPool{device, BUFFER_POOL_BLOCK_SIZE * 1024, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT}, nullptr));
	buffer_pools.emplace(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, std::make_pair(BufferPool{device, BUFFER_POOL_BLOCK_SIZE * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT}, nullptr));
	buffer_pools.emplace(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, std::make_pair(BufferPool{device, BUFFER_POOL_BLOCK_SIZE * 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT}, nullptr));
}

void RenderFrame::update_render_target(RenderTarget &&render_target)
{
	swapchain_render_target = std::move(render_target);
}

void RenderFrame::reset()
{
	fence_pool.wait();

	fence_pool.reset();

	for (auto &command_pools_per_queue : command_pools)
	{
		for (auto &command_pool : command_pools_per_queue.second)
		{
			command_pool->reset_pool();
		}
	}

	for (auto &buffer_pool_it : buffer_pools)
	{
		auto &buffer_pool  = buffer_pool_it.second.first;
		auto *buffer_block = buffer_pool_it.second.second;

		buffer_pool.reset();

		buffer_block = nullptr;
	}

	semaphore_pool.reset();
}

std::vector<std::unique_ptr<CommandPool>> &RenderFrame::get_command_pools(const Queue &queue, CommandBuffer::ResetMode reset_mode)
{
	auto command_pool_it = command_pools.find(queue.get_family_index());

	if (command_pool_it != command_pools.end())
	{
		if (command_pool_it->second.at(0)->get_reset_mode() != reset_mode)
		{
			device.wait_idle();

			// Delete pools
			command_pools.erase(command_pool_it);
		}
		else
		{
			return command_pool_it->second;
		}
	}

	std::vector<std::unique_ptr<CommandPool>> queue_command_pools;
	for (int i = 0; i < command_pool_count; i++)
	{
		queue_command_pools.push_back(std::make_unique<CommandPool>(device, queue.get_family_index(), reset_mode));
	}

	auto res_ins_it = command_pools.emplace(queue.get_family_index(), std::move(queue_command_pools));

	if (!res_ins_it.second)
	{
		throw std::runtime_error("Failed to insert command pool");
	}

	command_pool_it = res_ins_it.first;

	return command_pool_it->second;
}

const FencePool &RenderFrame::get_fence_pool() const
{
	return fence_pool;
}

VkFence RenderFrame::request_fence()
{
	return fence_pool.request_fence();
}

const SemaphorePool &RenderFrame::get_semaphore_pool() const
{
	return semaphore_pool;
}

VkSemaphore RenderFrame::request_semaphore()
{
	return semaphore_pool.request_semaphore();
}

RenderTarget &RenderFrame::get_render_target()
{
	return swapchain_render_target;
}

const RenderTarget &RenderFrame::get_render_target_const() const
{
	return swapchain_render_target;
}

BufferAllocation RenderFrame::allocate_buffer(const VkBufferUsageFlags usage, const VkDeviceSize size)
{
	// Find a pool for this usage
	auto buffer_pool_it = buffer_pools.find(usage);
	if (buffer_pool_it == buffer_pools.end())
	{
		LOGE("No buffer pool for buffer usage {}", usage);
		return BufferAllocation{};
	}

	auto &buffer_pool  = buffer_pool_it->second.first;
	auto &buffer_block = buffer_pool_it->second.second;

	if (!buffer_block)
	{
		// If there is no block associated with the pool
		// Request one with that size
		buffer_block = &buffer_pool.request_buffer_block(to_u32(size));
	}

	auto data = buffer_block->allocate(to_u32(size));

	// Check if the buffer block can allocate the requested size
	if (data.empty())
	{
		buffer_block = &buffer_pool.request_buffer_block(to_u32(size));

		data = buffer_block->allocate(to_u32(size));
	}

	return data;
}
}        // namespace vkb
