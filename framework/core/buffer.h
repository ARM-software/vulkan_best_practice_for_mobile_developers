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

namespace vkb
{
class Device;

namespace core
{
class Buffer : public NonCopyable
{
  public:
	Buffer(Device &device, VkDeviceSize size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, VmaAllocationCreateFlags flags);

	Buffer(Buffer &&other);

	~Buffer();

	const Device &get_device() const;

	VkBuffer get_handle() const;

	VmaAllocation get_memory() const;

	/**
	 * @brief Maps vulkan memory to an host visible address
	 * @return Pointer to host visible memory
	 */
	uint8_t *map();

	/**
	 * @brief Unmaps vulkan memory from the host visible address
	 */
	void unmap();

	/**
	 * @brief Flushes if memory is HOST_VISIBLE or it is not HOST_COHERENT
	 */
	void flush() const;

	/**
	 * @return The size of the buffer
	 */
	VkDeviceSize get_size() const;

	/**
	 * @brief Updates the content of the buffer
	 * @param offset Offset from which to start uploading
	 * @param data Data to upload
	 */
	void update(size_t offset, const std::vector<uint8_t> &data);

	template <class T>
	void update(size_t offset, const T &value)
	{
		update(offset, reinterpret_cast<const uint8_t *>(&value), sizeof(T));
	}

	const uint8_t *get_data() const
	{
		return mapped_data;
	}

  private:
	void update(size_t offset, const uint8_t *src, size_t size);

	Device &device;

	VkBuffer handle{VK_NULL_HANDLE};

	VmaAllocation memory{VK_NULL_HANDLE};

	VkDeviceSize size{0};

	uint8_t *mapped_data{nullptr};

	/// Whether it has been mapped with vmaMapMemory
	bool mapped = false;
};
}        // namespace core
}        // namespace vkb
