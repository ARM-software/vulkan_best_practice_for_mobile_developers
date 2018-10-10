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

#include "frame.h"

namespace vkb
{
Frame::Properties::Properties(VkDevice device, uint32_t graphics_queue_index) :
    device(device),
    queue_index(graphics_queue_index)
{
	VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(device, &info, nullptr, &queue_submit_fence);
};

Frame::Properties::~Properties()
{
	if (queue_submit_fence != VK_NULL_HANDLE)
	{
		vkDestroyFence(device, queue_submit_fence, nullptr);
	}
	if (swapchain_acquire_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(device, swapchain_acquire_semaphore, nullptr);
	}
	if (swapchain_release_semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(device, swapchain_release_semaphore, nullptr);
	}
	if (primary_command_pool)
	{
		vkDestroyCommandPool(device, primary_command_pool, nullptr);
	}
}

Frame::Frame(std::shared_ptr<Device> device,
             uint32_t                graphics_queue_index,
             VkImage                 image,
             VkFormat                format,
             uint32_t                width,
             uint32_t                height)
{
	this->color_attachment = Image(device, width, height, image, format);
	this->extent.width     = width;
	this->extent.height    = height;
	this->properties       = std::unique_ptr<Frame::Properties>(new Frame::Properties(device->get_handle(), graphics_queue_index));
}

void Frame::build(std::shared_ptr<Device> device, VkRenderPass render_pass)
{
	/// Allocate attachments
	std::vector<VkImageView> attachments;
	attachments.resize(this->image_attachments.size() + 1);

	/// Fill attachment view data
	attachments[0] = this->color_attachment.view;
	for (size_t i = 0; i < this->image_attachments.size(); i++)
	{
		attachments[i + 1] = this->image_attachments[i].view;
	}

	/// Create framebuffer
	VkFramebufferCreateInfo fb_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	fb_info.renderPass              = render_pass;
	fb_info.attachmentCount         = attachments.size();
	fb_info.pAttachments            = attachments.data();
	fb_info.width                   = this->color_attachment.extent.width;
	fb_info.height                  = this->color_attachment.extent.height;
	fb_info.layers                  = 1;

	VK_CHECK(vkCreateFramebuffer(device->get_handle(), &fb_info, nullptr, &this->handle));
}

void Frame::attach(Image &image)
{
	image_attachments.push_back(image);
}

void Frame::cleanup(std::shared_ptr<Device> device, bool destroy_image)
{
	this->extent = {};
	this->color_attachment.cleanup(destroy_image);

	for (auto image : this->image_attachments)
	{
		image.cleanup();
	}

	vkDestroyFramebuffer(device->get_handle(), this->handle, nullptr);

	device     = nullptr;
	properties = nullptr;
}

VkCommandBuffer Frame::request_command_buffer(bool postprocess_cmd)
{
	VkCommandBuffer cmd;

	if (!postprocess_cmd)
	{
		cmd = properties->primary_command_buffer;
	}
	else
	{
		cmd = properties->postprocessing_command_buffer;
	}

	if (cmd == VK_NULL_HANDLE)
	{
		if (properties->primary_command_pool == VK_NULL_HANDLE)
		{
			VkCommandPoolCreateInfo info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
			info.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			info.queueFamilyIndex        = properties->queue_index;

			VkCommandPool cmd_pool;
			VK_CHECK(vkCreateCommandPool(properties->device, &info, nullptr, &cmd_pool));
			properties->primary_command_pool = cmd_pool;
		}

		VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		info.commandPool                 = properties->primary_command_pool;
		info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount          = 1;
		VK_CHECK(vkAllocateCommandBuffers(properties->device, &info, &cmd));
		if (!postprocess_cmd)
		{
			properties->primary_command_buffer = cmd;
		}
		else
		{
			properties->postprocessing_command_buffer = cmd;
		}
	}

	return cmd;
}
}        // namespace vkb
