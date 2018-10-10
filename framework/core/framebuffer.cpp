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

#include "framebuffer.h"

#include "device.h"

namespace vkb
{
VkFramebuffer Framebuffer::get_handle() const
{
	return handle;
}

Framebuffer::Framebuffer(Device &device, const RenderTarget &render_target, const RenderPass &render_pass) :
    device{device}
{
	auto &extent = render_target.get_extent();

	std::vector<VkImageView> attachments;

	for (auto &view : render_target.get_views())
	{
		attachments.emplace_back(view.get_handle());
	}

	VkFramebufferCreateInfo create_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};

	create_info.renderPass      = render_pass.get_handle();
	create_info.attachmentCount = to_u32(attachments.size());
	create_info.pAttachments    = attachments.data();
	create_info.width           = extent.width;
	create_info.height          = extent.height;
	create_info.layers          = 1;

	auto result = vkCreateFramebuffer(device.get_handle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create Framebuffer"};
	}
}

Framebuffer::Framebuffer(Framebuffer &&other) :
    device{other.device},
    handle{other.handle}
{
	other.handle = VK_NULL_HANDLE;
}

Framebuffer::~Framebuffer()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device.get_handle(), handle, nullptr);
	}
}
}        // namespace vkb
