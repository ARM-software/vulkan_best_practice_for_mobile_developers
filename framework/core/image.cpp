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

#include "image.h"

#include "device.h"

namespace vkb
{
namespace
{
inline VkImageType find_image_type(VkExtent3D extent)
{
	VkImageType result{};

	uint32_t dim_num{0};

	if (extent.width > 1)
	{
		dim_num++;
	}

	if (extent.height > 1)
	{
		dim_num++;
	}

	if (extent.depth > 1)
	{
		dim_num++;
	}

	switch (dim_num)
	{
		case 1:
			result = VK_IMAGE_TYPE_1D;
			break;
		case 2:
			result = VK_IMAGE_TYPE_2D;
			break;
		case 3:
			result = VK_IMAGE_TYPE_3D;
			break;
		default:
			throw std::runtime_error("No image type found.");
			break;
	}

	return result;
}
}        // namespace

namespace core
{
Image::Image(Device &              device,
             const VkExtent3D &    extent,
             VkFormat              format,
             VkImageUsageFlags     image_usage,
             VmaMemoryUsage        memory_usage,
             VkSampleCountFlagBits sample_count,
             uint32_t              mip_levels,
             uint32_t              array_layers) :
    device{device},
    type{find_image_type(extent)},
    extent{extent},
    format{format},
    samples{sample_count},
    mip_levels{mip_levels},
    array_layers{array_layers}
{
	VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

	image_info.imageType   = type;
	image_info.format      = format;
	image_info.extent      = extent;
	image_info.samples     = samples;
	image_info.usage       = image_usage;
	image_info.mipLevels   = mip_levels;
	image_info.arrayLayers = array_layers;

	VmaAllocationCreateInfo memory_info{};
	memory_info.usage = memory_usage;

	if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
	{
		memory_info.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
	}

	auto result = vmaCreateImage(device.get_memory_allocator(),
	                             &image_info, &memory_info,
	                             &handle, &memory,
	                             nullptr);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create Image"};
	}
}

Image::Image(Device &device, VkImage handle, const VkExtent3D &extent, VkFormat format) :
    device{device},
    handle{handle},
    type{find_image_type(extent)},
    extent{extent},
    format{format},
    samples{VK_SAMPLE_COUNT_1_BIT}
{}

Image::Image(Image &&other) :
    device{other.device},
    handle{other.handle},
    memory{other.memory},
    type{other.type},
    extent{other.extent},
    format{other.format},
    samples{other.samples}
{
	other.handle = VK_NULL_HANDLE;
	other.memory = VK_NULL_HANDLE;
}

Image::~Image()
{
	if (handle != VK_NULL_HANDLE && memory != VK_NULL_HANDLE)
	{
		vmaDestroyImage(device.get_memory_allocator(), handle, memory);
	}
}

const Device &Image::get_device() const
{
	return device;
}

VkImage Image::get_handle() const
{
	return handle;
}

VmaAllocation Image::get_memory() const
{
	return memory;
}

VkImageType Image::get_type() const
{
	return type;
}

const VkExtent3D &Image::get_extent() const
{
	return extent;
}

VkFormat Image::get_format() const
{
	return format;
}

VkSampleCountFlagBits Image::get_samples() const
{
	return samples;
}

uint32_t Image::get_mip_levels() const
{
	return mip_levels;
}

uint32_t Image::get_array_layers() const
{
	return array_layers;
}

}        // namespace core
}        // namespace vkb
