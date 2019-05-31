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
class Image : public NonCopyable
{
  public:
	Image(Device &          device,
	      VkImage           handle,
	      const VkExtent3D &extent,
	      VkFormat          format,
	      VkImageUsageFlags image_usage);

	Image(Device &              device,
	      const VkExtent3D &    extent,
	      VkFormat              format,
	      VkImageUsageFlags     image_usage,
	      VmaMemoryUsage        memory_usage,
	      VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
	      uint32_t              mip_levels   = 1,
	      uint32_t              array_layers = 1);

	Image(Image &&other);

	~Image();

	const Device &get_device() const;

	VkImage get_handle() const;

	VmaAllocation get_memory() const;

	VkImageType get_type() const;

	const VkExtent3D &get_extent() const;

	VkFormat get_format() const;

	VkSampleCountFlagBits get_samples() const;

	uint32_t get_mip_levels() const;

	uint32_t get_array_layers() const;

	VkImageUsageFlags get_usage() const;

  private:
	Device &device;

	VkImage handle{VK_NULL_HANDLE};

	VmaAllocation memory{VK_NULL_HANDLE};

	VkImageType type{};

	VkExtent3D extent{};

	VkFormat format{};

	VkImageUsageFlags usage{};

	VkSampleCountFlagBits sample_count{};

	uint32_t mip_levels{1};

	uint32_t array_layers{1};
};
}        // namespace core
}        // namespace vkb
