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

namespace vkb
{
class Device;

namespace core
{
class Buffer : public NonCopyable
{
  public:
	Buffer(Device &device, VkDeviceSize size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage);

	Buffer(Buffer &&other);

	~Buffer();

	const Device &get_device() const;

	VkBuffer get_handle() const;

	VmaAllocation get_memory() const;

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
		update(offset, std::vector<uint8_t>{reinterpret_cast<const uint8_t *>(&value),
		                                    reinterpret_cast<const uint8_t *>(&value) + sizeof(T)});
	}

	const uint8_t *get_data() const
	{
		return mapped_data;
	}

  private:
	Device &device;

	VkBuffer handle{VK_NULL_HANDLE};

	VmaAllocation memory{VK_NULL_HANDLE};

	VkDeviceSize size{0};

	uint8_t *mapped_data{nullptr};
};
}        // namespace core
}        // namespace vkb
