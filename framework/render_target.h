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

#include "core/image.h"
#include "core/image_view.h"

namespace vkb
{
class Device;

struct Attachment
{
	VkFormat format;

	VkSampleCountFlagBits samples;

	VkImageUsageFlags usage;

	Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT);
};

class RenderTarget : public NonCopyable
{
  public:
	RenderTarget(Device &device, std::vector<core::Image> &&images);

	RenderTarget(Device &device, const VkExtent2D &extent, const std::vector<Attachment> &attachments);

	const VkExtent2D &get_extent() const;

	const std::vector<ImageView> &get_views() const;

	const std::vector<Attachment> &get_attachments() const;

  private:
	Device &device;

	VkExtent2D extent{};

	std::vector<core::Image> images;

	std::vector<ImageView> views;

	std::vector<Attachment> attachments;
};
}        // namespace vkb
