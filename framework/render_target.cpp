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

#include "render_target.h"

namespace vkb
{
namespace
{
struct CompareExtent2D : public std::binary_function<VkExtent2D, VkExtent2D, bool>
{
	bool operator()(const VkExtent2D &lhs, const VkExtent2D &rhs) const
	{
		return !(lhs.width == rhs.width && lhs.height == rhs.height) && (lhs.width < rhs.width && lhs.height < rhs.height);
	}
};
}        // namespace

Attachment::Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) :
    format{format},
    samples{samples},
    usage{usage}
{
}

vkb::RenderTarget::RenderTarget(Device &device, std::vector<core::Image> &&images) :
    device{device},
    images{std::move(images)}
{
	std::set<VkExtent2D, CompareExtent2D> unique_extent;

	// Returns the image extent as a VkExtent2D structure from a VkExtent3D
	auto get_image_extent = [](const core::Image &image) { return VkExtent2D{image.get_extent().width, image.get_extent().height}; };

	// Constructs a set of unique image extens given a vector of images
	std::transform(this->images.begin(), this->images.end(), std::inserter(unique_extent, unique_extent.end()), get_image_extent);

	// Allow only one extent size for a render target
	if (unique_extent.size() != 1)
	{
		throw VulkanException{VK_ERROR_INITIALIZATION_FAILED, "Extent size is not unique"};
	}

	extent = *unique_extent.begin();

	for (auto &image : this->images)
	{
		if (image.get_type() != VK_IMAGE_TYPE_2D)
		{
			throw VulkanException{VK_ERROR_INITIALIZATION_FAILED, "Image type is not 2D"};
		}

		views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);

		attachments.emplace_back(Attachment{image.get_format(), image.get_samples()});
	}
}

RenderTarget::RenderTarget(Device &device, const VkExtent2D &extent, const std::vector<Attachment> &attachments) :
    device{device},
    extent{extent},
    attachments{attachments}
{
	VkExtent3D image_extent = {extent.width, extent.height, 1};

	for (auto &attachment : attachments)
	{
		VkImageUsageFlags usage = attachment.usage;

		if (is_depth_stencil_format(attachment.format))
		{
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		images.emplace_back(device, image_extent, attachment.format,
		                    usage, VMA_MEMORY_USAGE_GPU_ONLY);
	}

	for (auto &image : images)
	{
		views.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
	}
}

const VkExtent2D &RenderTarget::get_extent() const
{
	return extent;
}

const std::vector<ImageView> &RenderTarget::get_views() const
{
	return views;
}

const std::vector<Attachment> &RenderTarget::get_attachments() const
{
	return attachments;
}

}        // namespace vkb
