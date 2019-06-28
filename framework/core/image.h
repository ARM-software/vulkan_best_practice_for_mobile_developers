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

#include <unordered_set>

#include "common/helpers.h"
#include "common/vk_common.h"

namespace vkb
{
class Device;

namespace core
{
class ImageView;
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
	      uint32_t              array_layers = 1,
	      VkImageTiling         tiling       = VK_IMAGE_TILING_OPTIMAL);

	Image(Image &&other);

	~Image();

	Device &get_device();

	VkImage get_handle() const;

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

	VkImageType get_type() const;

	const VkExtent3D &get_extent() const;

	VkFormat get_format() const;

	VkSampleCountFlagBits get_sample_count() const;

	VkImageUsageFlags get_usage() const;

	VkImageTiling get_tiling() const;

	VkImageSubresource get_subresource() const;

	std::unordered_set<ImageView *> &get_views();

  private:
	Device &device;

	VkImage handle{VK_NULL_HANDLE};

	VmaAllocation memory{VK_NULL_HANDLE};

	VkImageType type{};

	VkExtent3D extent{};

	VkFormat format{};

	VkImageUsageFlags usage{};

	VkSampleCountFlagBits sample_count{};

	VkImageTiling tiling{};

	VkImageSubresource subresource{};

	/// Image views referring to this image
	std::unordered_set<ImageView *> views;

	uint8_t *mapped_data{nullptr};

	/// Whether it was mapped with vmaMapMemory
	bool mapped{false};
};
}        // namespace core
}        // namespace vkb
