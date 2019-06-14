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

namespace vkb
{
class ImageView : public NonCopyable
{
  public:
	ImageView(core::Image &image, VkImageViewType view_type, VkFormat format = VK_FORMAT_UNDEFINED);

	ImageView(ImageView &&other);

	~ImageView();

	const core::Image &get_image() const;

	/**
	 * @brief Update the image this view is referring to
	 *        Used on image move
	 */
	void set_image(core::Image &image);

	VkImageView get_handle() const;

	VkFormat get_format() const;

	VkImageSubresourceRange get_subresource_range() const;

	VkImageSubresourceLayers get_subresource_layers() const;

  private:
	Device &device;

	core::Image *image{};

	VkImageView handle{VK_NULL_HANDLE};

	VkFormat format{};

	VkImageSubresourceRange subresource_range{};
};
}        // namespace vkb
