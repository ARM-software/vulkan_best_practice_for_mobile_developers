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

#include "image_view.h"

#include "core/image.h"
#include "device.h"

namespace vkb
{
ImageView::ImageView(core::Image &img, VkImageViewType view_type, VkFormat format) :
    device{img.get_device()},
    image{&img},
    format{format}
{
	if (format == VK_FORMAT_UNDEFINED)
	{
		this->format = format = image->get_format();
	}

	subresource_range.levelCount = image->get_mip_levels();
	subresource_range.layerCount = image->get_array_layers();

	if (is_depth_only_format(format))
	{
		subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (is_depth_stencil_format(format))
	{
		subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

	view_info.image            = image->get_handle();
	view_info.viewType         = view_type;
	view_info.format           = format;
	view_info.subresourceRange = subresource_range;

	auto result = vkCreateImageView(device.get_handle(), &view_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create ImageView"};
	}

	// Register this image view to its image
	// in order to be notified when it gets moved
	image->get_views().emplace(this);
}

ImageView::ImageView(ImageView &&other) :
    device{other.device},
    image{other.image},
    handle{other.handle},
    format{other.format},
    subresource_range{other.subresource_range}
{
	// Remove old view from image set and add this new one
	auto &views = image->get_views();
	views.erase(&other);
	views.emplace(this);

	other.handle = VK_NULL_HANDLE;
}

ImageView::~ImageView()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device.get_handle(), handle, nullptr);
	}
}

const core::Image &ImageView::get_image() const
{
	assert(image && "Image view is referring an invalid image");
	return *image;
}

void ImageView::set_image(core::Image &img)
{
	image = &img;
}

VkImageView ImageView::get_handle() const
{
	return handle;
}

VkFormat ImageView::get_format() const
{
	return format;
}

VkImageSubresourceRange ImageView::get_subresource_range() const
{
	return subresource_range;
}

VkImageSubresourceLayers ImageView::get_subresource_layers() const
{
	VkImageSubresourceLayers subresource{};
	subresource.aspectMask     = subresource_range.aspectMask;
	subresource.baseArrayLayer = subresource_range.baseArrayLayer;
	subresource.layerCount     = subresource_range.layerCount;
	subresource.mipLevel       = subresource_range.baseMipLevel;
	return subresource;
}

}        // namespace vkb
