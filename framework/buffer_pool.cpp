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

#include "buffer_pool.h"

#include <cstddef>

#include "common/error.h"
#include "common/logging.h"

namespace vkb
{
BufferBlock::BufferBlock(Device &device, VkDeviceSize size, VkBufferUsageFlags usage) :
    buffer{device, size, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT}
{
	if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		alignment = device.get_properties().limits.minUniformBufferOffsetAlignment;
	}
	else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	{
		alignment = device.get_properties().limits.minStorageBufferOffsetAlignment;
	}
	else
	{
		alignment = 16;
	}
}

BufferAllocation BufferBlock::allocate(const uint32_t allocate_size)
{
	assert(allocate_size > 0 && "Allocation size must be greater than zero");

	auto aligned_offset = (offset + alignment - 1) & ~(alignment - 1);

	if (aligned_offset + allocate_size > buffer.get_size())
	{
		// No more space available from the underlying buffer, return empty allocation
		return BufferAllocation{};
	}

	// Move the current offset and return an allocation
	offset = aligned_offset + allocate_size;
	return BufferAllocation{buffer, allocate_size, aligned_offset};
}

VkDeviceSize BufferBlock::get_size() const
{
	return buffer.get_size();
}

void BufferBlock::reset()
{
	offset = 0;
}

BufferPool::BufferPool(Device &device, VkDeviceSize block_size, VkBufferUsageFlags usage) :
    device{device},
    block_size{block_size},
    usage{usage}
{
}

BufferBlock &BufferPool::request_buffer_block(const VkDeviceSize minimum_size)
{
	// Find the first block in the range of the inactive blocks
	// which size is greater than the minimum size
	auto it = std::upper_bound(buffer_blocks.begin() + active_buffer_block_count, buffer_blocks.end(), minimum_size,
	                           [](const VkDeviceSize &a, const BufferBlock &b) -> bool { return a < b.get_size(); });

	if (it != buffer_blocks.end())
	{
		// Recycle inactive block
		active_buffer_block_count++;
		return *it;
	}

	LOGI("Building #{} buffer block ({})", buffer_blocks.size(), usage);

	// Create a new block, store and return it
	auto &block = buffer_blocks.emplace_back(BufferBlock{device, std::max(block_size, minimum_size), usage});

	active_buffer_block_count++;

	return block;
}

void BufferPool::reset()
{
	for (auto &buffer_block : buffer_blocks)
	{
		buffer_block.reset();
	}

	active_buffer_block_count = 0;
}

BufferAllocation::BufferAllocation(core::Buffer &buffer, VkDeviceSize size, VkDeviceSize offset) :
    buffer{&buffer},
    size{size},
    base_offset{offset}
{
}

void BufferAllocation::update(uint32_t offset, const std::vector<uint8_t> &data)
{
	assert(buffer && "Invalid buffer pointer");

	if (offset + data.size() <= size)
	{
		buffer->update(static_cast<size_t>(base_offset) + offset, data);
	}
	else
	{
		LOGE("Ignore buffer allocation update");
	}
}

bool BufferAllocation::empty() const
{
	return size == 0 || buffer == nullptr;
}

VkDeviceSize BufferAllocation::get_size() const
{
	return size;
}

VkDeviceSize BufferAllocation::get_offset() const
{
	return base_offset;
}

core::Buffer &BufferAllocation::get_buffer()
{
	assert(buffer && "Invalid buffer pointer");
	return *buffer;
}

}        // namespace vkb
