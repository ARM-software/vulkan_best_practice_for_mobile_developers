/* Copyright (c) 2018-2019, Arm Limited and Contributors
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

#include <mutex>

#include "scene_graph/components/image/astc.h"
#include "scene_graph/components/image/ktx.h"
#include "scene_graph/components/image/stb.h"

#include "platform/file.h"
#include "utils.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

namespace vkb
{
namespace sg
{
bool is_astc(const VkFormat format)
{
	return (format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_4x4_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_5x4_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_5x4_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_5x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_5x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_6x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_6x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_6x6_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_6x6_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_8x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_8x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_8x6_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_8x6_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_8x8_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_8x8_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x5_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x5_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x6_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x6_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x8_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x8_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_10x10_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_10x10_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_12x10_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_12x10_SRGB_BLOCK ||
	        format == VK_FORMAT_ASTC_12x12_UNORM_BLOCK ||
	        format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
}

Image::Image(const std::string &name, std::vector<uint8_t> &&d, std::vector<Mipmap> &&m) :
    Component{name},
    data{std::move(d)},
    format{VK_FORMAT_R8G8B8A8_UNORM},
    mipmaps{std::move(m)}
{
}

std::type_index Image::get_type()
{
	return typeid(Image);
}

const std::vector<uint8_t> &Image::get_data() const
{
	return data;
}

void Image::clear_data()
{
	data.clear();
	data.shrink_to_fit();
}

VkFormat Image::get_format() const
{
	return format;
}

const VkExtent3D &Image::get_extent() const
{
	return mipmaps.at(0).extent;
}

const std::vector<Mipmap> &Image::get_mipmaps() const
{
	return mipmaps;
}

void Image::create_vk_image(Device &device)
{
	assert(!vk_image && !vk_image_view && "Vulkan image already constructed");

	vk_image = std::make_unique<core::Image>(device,
	                                         get_extent(),
	                                         format,
	                                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	                                         VMA_MEMORY_USAGE_GPU_ONLY, VK_SAMPLE_COUNT_1_BIT,
	                                         to_u32(mipmaps.size()));

	vk_image_view = std::make_unique<ImageView>(*vk_image, VK_IMAGE_VIEW_TYPE_2D);
}

const core::Image &Image::get_vk_image() const
{
	assert(vk_image && "Vulkan image was not created");
	return *vk_image;
}

const ImageView &Image::get_vk_image_view() const
{
	assert(vk_image_view && "Vulkan image view was not created");
	return *vk_image_view;
}

Mipmap &Image::get_mipmap(const size_t index)
{
	return mipmaps.at(index);
}

void Image::generate_mipmaps()
{
	assert(mipmaps.size() == 1 && "Mipmaps already generated");

	if (mipmaps.size() > 1)
	{
		return;        // Do not generate again
	}

	auto extent      = get_extent();
	auto next_width  = std::max<uint32_t>(1u, extent.width / 2);
	auto next_height = std::max<uint32_t>(1u, extent.height / 2);
	auto channels    = 4;
	auto next_size   = next_width * next_height * channels;

	while (true)
	{
		// Make space for next mipmap
		auto old_size = to_u32(data.size());
		data.resize(old_size + next_size);

		auto &prev_mipmap = mipmaps.back();
		// Update mipmaps
		Mipmap next_mipmap{};
		next_mipmap.level  = prev_mipmap.level + 1;
		next_mipmap.offset = old_size;
		next_mipmap.extent = {next_width, next_height, 1u};

		// Fill next mipmap memory
		stbir_resize_uint8(data.data() + prev_mipmap.offset, prev_mipmap.extent.width, prev_mipmap.extent.height, 0,
		                   data.data() + next_mipmap.offset, next_mipmap.extent.width, next_mipmap.extent.height, 0, channels);

		mipmaps.emplace_back(std::move(next_mipmap));

		// Next mipmap values
		next_width  = std::max<uint32_t>(1u, next_width / 2);
		next_height = std::max<uint32_t>(1u, next_height / 2);
		next_size   = next_width * next_height * channels;

		if (next_width == 1 && next_height == 1)
		{
			break;
		}
	}
}

std::vector<Mipmap> &Image::get_mut_mipmaps()
{
	return mipmaps;
}

std::vector<uint8_t> &Image::get_mut_data()
{
	return data;
}

void Image::set_data(const uint8_t *raw_data, size_t size)
{
	assert(data.empty() && "Image data already set");
	data = {raw_data, raw_data + size};
}

void Image::set_format(const VkFormat f)
{
	format = f;
}

void Image::set_width(const uint32_t width)
{
	mipmaps.at(0).extent.width = width;
}

void Image::set_height(const uint32_t height)
{
	mipmaps.at(0).extent.height = height;
}

void Image::set_depth(const uint32_t depth)
{
	mipmaps.at(0).extent.depth = depth;
}

std::unique_ptr<Image> Image::load(const std::string &name, const std::string &uri)
{
	std::unique_ptr<Image> image{nullptr};

	auto data = file::read_asset(uri);

	// Get extension
	auto extension = get_extension(uri);

	if (extension == "png" || extension == "jpg")
	{
		image = std::make_unique<Stb>(name, data);
	}
	else if (extension == "astc")
	{
		image = std::make_unique<Astc>(name, data);
	}
	else if (extension == "ktx")
	{
		image = std::make_unique<Ktx>(name, data);
	}

	return image;
}

}        // namespace sg
}        // namespace vkb
