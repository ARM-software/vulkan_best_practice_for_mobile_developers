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

#include "render_pass.h"

#include "device.h"
#include "render_target.h"

namespace vkb
{
VkRenderPass RenderPass::get_handle() const
{
	return handle;
}

RenderPass::RenderPass(Device &device, const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses) :
    device{device}
{
	uint32_t depth_stencil_attachment{VK_ATTACHMENT_UNUSED};

	std::vector<VkAttachmentDescription> attachment_descriptions;

	for (uint32_t i = 0U; i < attachments.size(); ++i)
	{
		VkAttachmentDescription attachment{};

		attachment.format  = attachments[i].format;
		attachment.samples = attachments[i].samples;

		if (i < load_store_infos.size())
		{
			attachment.loadOp         = load_store_infos[i].load_op;
			attachment.storeOp        = load_store_infos[i].store_op;
			attachment.stencilLoadOp  = load_store_infos[i].load_op;
			attachment.stencilStoreOp = load_store_infos[i].store_op;
		}

		if (is_depth_stencil_format(attachment.format))
		{
			depth_stencil_attachment = i;
		}

		attachment_descriptions.push_back(std::move(attachment));
	}

	std::vector<VkSubpassDescription>  subpass_descriptions;
	std::vector<VkAttachmentReference> input_attachments;
	std::vector<VkAttachmentReference> color_attachments;
	std::vector<VkAttachmentReference> depth_stencil_attachments;

	for (uint32_t i = 0U; i < subpasses.size(); ++i)
	{
		VkSubpassDescription subpass_description{};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (uint32_t k : subpasses[i].input_attachments)
		{
			if (is_depth_stencil_format(attachment_descriptions[k].format))
			{
				input_attachments.push_back({k, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL});
			}
			else
			{
				input_attachments.push_back({k, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
			}

			if (!subpass_description.pInputAttachments)
			{
				subpass_description.pInputAttachments = &(*input_attachments.rbegin());
			}
		}

		subpass_description.inputAttachmentCount = to_u32(subpasses[i].input_attachments.size());

		for (auto k : subpasses[i].output_attachments)
		{
			if (k == depth_stencil_attachment)
			{
				continue;
			}

			color_attachments.push_back({k, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

			if (!subpass_description.pColorAttachments)
			{
				subpass_description.pColorAttachments = &(*color_attachments.rbegin());
			}
		}

		if (depth_stencil_attachment != VK_ATTACHMENT_UNUSED)
		{
			depth_stencil_attachments.push_back({depth_stencil_attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

			subpass_description.pDepthStencilAttachment = &(*depth_stencil_attachments.rbegin());
		}

		subpass_description.colorAttachmentCount = to_u32(subpasses[i].output_attachments.size());

		subpass_descriptions.push_back(subpass_description);
	}

	if (subpasses.empty())
	{
		VkSubpassDescription subpass_description{};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (uint32_t k = 0U; k < attachment_descriptions.size(); ++k)
		{
			if (k == depth_stencil_attachment)
			{
				continue;
			}

			color_attachments.push_back({k, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

			if (!subpass_description.pColorAttachments)
			{
				subpass_description.pColorAttachments = &(*color_attachments.rbegin());
			}
		}

		if (depth_stencil_attachment != VK_ATTACHMENT_UNUSED)
		{
			depth_stencil_attachments.push_back({depth_stencil_attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});

			subpass_description.pDepthStencilAttachment = &(*depth_stencil_attachments.rbegin());
		}

		subpass_descriptions.push_back(subpass_description);
	}

	// Make the initial layout same as the first subpass layout
	{
		const VkSubpassDescription &subpass = subpass_descriptions.front();

		for (uint32_t k = 0U; k < subpass.colorAttachmentCount; ++k)
		{
			VkAttachmentReference reference = subpass.pColorAttachments[k];

			attachment_descriptions[reference.attachment].initialLayout = reference.layout;
		}

		for (uint32_t k = 0U; k < subpass.inputAttachmentCount; ++k)
		{
			VkAttachmentReference reference = subpass.pInputAttachments[k];

			attachment_descriptions[reference.attachment].initialLayout = reference.layout;
		}

		if (subpass.pDepthStencilAttachment)
		{
			VkAttachmentReference reference = *subpass.pDepthStencilAttachment;

			attachment_descriptions[reference.attachment].initialLayout = reference.layout;
		}
	}

	// Make the final layout same as the last subpass layout
	{
		const VkSubpassDescription &subpass = subpass_descriptions.back();

		for (uint32_t k = 0U; k < subpass.colorAttachmentCount; ++k)
		{
			VkAttachmentReference reference = subpass.pColorAttachments[k];

			attachment_descriptions[reference.attachment].finalLayout = reference.layout;
		}

		for (uint32_t k = 0U; k < subpass.inputAttachmentCount; ++k)
		{
			VkAttachmentReference reference = subpass.pInputAttachments[k];

			attachment_descriptions[reference.attachment].finalLayout = reference.layout;
		}

		if (subpass.pDepthStencilAttachment)
		{
			VkAttachmentReference reference = *subpass.pDepthStencilAttachment;

			attachment_descriptions[reference.attachment].finalLayout = reference.layout;
		}
	}

	VkRenderPassCreateInfo create_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

	create_info.attachmentCount = to_u32(attachment_descriptions.size());
	create_info.pAttachments    = attachment_descriptions.data();
	create_info.subpassCount    = to_u32(subpass_descriptions.size());
	create_info.pSubpasses      = subpass_descriptions.data();

	auto result = vkCreateRenderPass(device.get_handle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create RenderPass"};
	}
}        // namespace vkb

RenderPass::RenderPass(RenderPass &&other) :
    device{other.device},
    handle{other.handle}
{
	other.handle = VK_NULL_HANDLE;
}

RenderPass::~RenderPass()
{
	// Destroy render pass
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device.get_handle(), handle, nullptr);
	}
}
}        // namespace vkb
