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
struct Attachment;
class Device;

struct LoadStoreInfo
{
	VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;

	VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
};

struct SubpassInfo
{
	std::vector<uint32_t> input_attachments;

	std::vector<uint32_t> output_attachments;
};

class RenderPass
{
  public:
	VkRenderPass get_handle() const;

	RenderPass(Device &                          device,
	           const std::vector<Attachment> &   attachemnts,
	           const std::vector<LoadStoreInfo> &load_store_infos,
	           const std::vector<SubpassInfo> &  subpasses);

	RenderPass(const RenderPass &) = delete;

	RenderPass(RenderPass &&other);

	~RenderPass();

	RenderPass &operator=(const RenderPass &) = delete;

	RenderPass &operator=(RenderPass &&) = delete;

	const uint32_t get_color_output_count(uint32_t subpass_index) const;

  private:
	Device &device;

	VkRenderPass handle{VK_NULL_HANDLE};

	size_t subpass_count;

	// Store attachments for every subpass
	std::vector<std::vector<VkAttachmentReference>> input_attachments;

	std::vector<std::vector<VkAttachmentReference>> color_attachments;

	std::vector<std::vector<VkAttachmentReference>> depth_stencil_attachments;
};
}        // namespace vkb
