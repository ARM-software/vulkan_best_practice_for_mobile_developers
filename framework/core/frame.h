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
#include "image.h"

namespace vkb
{
class Frame
{
  public:
	/// @brief Constructs a Frame class
	/// @param device A Vulkan device
	/// @param graphics_queue_index The index of the graphics queue
	/// @param image The swapchain image to construct the framebuffer from
	/// @param format The format of the swapchain
	/// @param width The swapchains extent width
	/// @param height The swapchains extent height
	Frame()
	{}
	Frame(std::shared_ptr<Device> device,
	      uint32_t                graphics_queue_index,
	      VkImage                 image,
	      VkFormat                format,
	      uint32_t                width,
	      uint32_t                height);

	/// @brief Destructs a frame class
	void cleanup(std::shared_ptr<Device> device, bool destroy_image = true);

	/// @brief Gets the handle to the Frame
	/// @returns VkFramebuffer
	VkFramebuffer get()
	{
		return handle;
	}

	/// @brief Builds the framebuffer out of the image attachments.
	/// @param device The Vulkan device
	/// @param render_pass A valid Vulkan render pass
	void build(std::shared_ptr<Device> device, VkRenderPass render_pass);

	/// @brief Attaches an image to the framebuffer image attachments
	/// @param image The Image to be attached.
	void attach(Image &image);

	/// @brief Gets a command buffer
	/// @param postprocess Whether the command should be for post processing or not
	/// @returns A valid command buffer
	VkCommandBuffer request_command_buffer(bool postprocess = false);

	/// The handle to the framebuffer
	VkFramebuffer handle = VK_NULL_HANDLE;

	/// The height and width of the frame
	VkExtent2D extent = {};

	/// The base image colour buffer
	Image color_attachment;

	/// The image attachments the framebuffer contains
	std::vector<Image> image_attachments;

	/// Per frame properties
	struct Properties
	{
		VkDevice        device                        = VK_NULL_HANDLE;
		VkFence         queue_submit_fence            = VK_NULL_HANDLE;
		VkCommandPool   primary_command_pool          = VK_NULL_HANDLE;
		VkCommandBuffer primary_command_buffer        = VK_NULL_HANDLE;
		VkSemaphore     swapchain_acquire_semaphore   = VK_NULL_HANDLE;
		VkSemaphore     swapchain_release_semaphore   = VK_NULL_HANDLE;
		VkCommandBuffer postprocessing_command_buffer = VK_NULL_HANDLE;
		VkSemaphore     postprocessing_semaphore      = VK_NULL_HANDLE;
		uint32_t        queue_index;

		Properties(VkDevice device, uint32_t graphics_queue_index);
		~Properties();
	};
	std::unique_ptr<Frame::Properties> properties;
};
}        // namespace vkb
