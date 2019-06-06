/* Copyright (c) 2018-2019, Arm Limited and Contributors
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

#include "platform/application.h"

struct SwapchainDimensions
{
	// Width of the swapchain.
	uint32_t width = 0;

	// Height of the swapchain.
	uint32_t height = 0;

	// Pixel format of the swapchain.
	VkFormat format = VK_FORMAT_UNDEFINED;
};

/// Per-frame information for all the frames in flight.
struct PerFrame
{
	VkDevice device = VK_NULL_HANDLE;

	VkFence queue_submit_fence = VK_NULL_HANDLE;

	VkCommandPool primary_command_pool = VK_NULL_HANDLE;

	VkCommandBuffer primary_command_buffer = VK_NULL_HANDLE;

	VkSemaphore swapchain_acquire_semaphore = VK_NULL_HANDLE;

	VkSemaphore swapchain_release_semaphore = VK_NULL_HANDLE;

	int32_t queue_index;
};

struct Context
{
	/// The Vulkan instance.
	VkInstance instance = VK_NULL_HANDLE;

	/// The Vulkan physical device.
	VkPhysicalDevice gpu = VK_NULL_HANDLE;

	/// The Vulkan device.
	VkDevice device = VK_NULL_HANDLE;

	/// The Vulkan device queue.
	VkQueue queue = VK_NULL_HANDLE;

	/// The swapchain.
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;

	/// The swapchain dimensions.
	SwapchainDimensions swapchain_dimensions;

	/// The surface we will render to.
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	/// The queue family index where graphics work will be submitted.
	int32_t graphics_queue_index = -1;

	/// The image view for each swapchain image.
	std::vector<VkImageView> swapchain_image_views;

	/// The framebuffer for each swapchain image view.
	std::vector<VkFramebuffer> swapchain_framebuffers;

	// The renderpass description.
	VkRenderPass render_pass = VK_NULL_HANDLE;

	// The graphics pipeline.
	VkPipeline pipeline = VK_NULL_HANDLE;

	// Specified the pipeline layout for resources.
	// We don't use any in this sample, but we still need to provide a dummy one.
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

	/// The debug report callback.
	VkDebugReportCallbackEXT debug_callback = VK_NULL_HANDLE;

	/// A set of semaphores that can be reused.
	std::vector<VkSemaphore> recycled_semaphores;

	std::vector<PerFrame> per_frame;
};

class HelloTriangle : public vkb::Application
{
  public:
	HelloTriangle();

	virtual ~HelloTriangle();

	virtual bool prepare(vkb::Platform &platform) override;

	virtual void update(float delta_time) override;

	virtual void resize(const uint32_t width, const uint32_t height) override;

  private:
	Context context;
};

std::unique_ptr<vkb::Application> create_hello_triangle();
