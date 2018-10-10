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

class Swapchain : public NonCopyable
{
  public:
	/**
	 * @brief Constructor to create a swapchain by changing the extent 
	 *        only and preserving the configuration from the old swapchain.
	 */
	Swapchain(Swapchain &old_swapchain, const VkExtent2D &extent);

	/**
	 * @brief Constructor to create a swapchain by changing the image count 
	 *        only and preserving the configuration from the old swapchain.
	 */
	Swapchain(Swapchain &old_swapchain, const uint32_t image_count);

	/**
	 * @brief Constructor to create a swapchain by changing the image usage
	 * only and preserving the configuration from the old swapchain.
	 */
	Swapchain(Swapchain &old_swapchain, const std::set<VkImageUsageFlagBits> &image_usage_flags);

	/**
	 * @brief Constructor to create a swapchain by changing the extent 
	 *        and transform only and preserving the configuration from the old swapchain.
	 */
	Swapchain(Swapchain &swapchain, const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

	/**
	 * @brief Constructor to create a swapchain.
	 */
	Swapchain(Device &                            device,
	          VkSurfaceKHR                        surface,
	          const VkExtent2D &                  extent       = {},
	          const uint32_t                      image_count  = 3,
	          const VkSurfaceTransformFlagBitsKHR transform    = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	          const VkPresentModeKHR              present_mode = VK_PRESENT_MODE_FIFO_KHR,
	          const VkImageUsageFlags             image_usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	/**
	 * @brief Constructor to create a swapchain from the old swapchain
	 *        by configuring all parameters.
	 */
	Swapchain(Swapchain &                         old_swapchain,
	          Device &                            device,
	          VkSurfaceKHR                        surface,
	          const VkExtent2D &                  extent       = {},
	          const uint32_t                      image_count  = 3,
	          const VkSurfaceTransformFlagBitsKHR transform    = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	          const VkPresentModeKHR              present_mode = VK_PRESENT_MODE_FIFO_KHR,
	          const VkImageUsageFlags             image_usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	/**
	 * @brief Destructor
	 */
	~Swapchain();

	/**
	 * @brief Move constructor
	 */
	Swapchain(Swapchain &&other);

	VkSwapchainKHR get_handle() const;

	VkResult acquire_next_image(uint32_t &image_index, VkSemaphore image_acquired_semaphore, VkFence fence);

	const VkExtent2D &get_extent() const;

	const VkFormat &get_format() const;

	const std::vector<VkImage> &get_images() const;

	inline const VkSurfaceTransformFlagBitsKHR get_transform() const;

	inline const VkSurfaceKHR get_surface()
	{
		return surface;
	}

  private:
	Device &device;

	VkSurfaceKHR surface{VK_NULL_HANDLE};

	VkSwapchainKHR handle{VK_NULL_HANDLE};

	std::vector<VkImage> images;

	VkExtent2D extent{};

	VkFormat format{};

	uint32_t image_count{};

	VkSurfaceTransformFlagBitsKHR transform{};

	VkPresentModeKHR present_mode{};

	VkImageUsageFlags image_usage{};
};

const VkSurfaceTransformFlagBitsKHR Swapchain::get_transform() const
{
	return transform;
}
}        // namespace vkb
