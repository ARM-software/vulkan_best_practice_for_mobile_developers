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

#include "scene_graph/components/image/ktx.h"

#include "common/error.h"

VKBP_DISABLE_WARNINGS
#include <ktx.h>
#include <vk_format.h>
VKBP_ENABLE_WARNINGS

namespace vkb
{
namespace sg
{
/// Row padding is different between KTX (pad to 4) and Vulkan (none).
/// Also region->bufferOffset, i.e. the start of each image, has
/// to be a multiple of 4 and also a multiple of the element size.
static KTX_error_code KTXAPIENTRY optimal_tiling_callback(int mip_level,
                                                          int /*face*/,
                                                          int          width,
                                                          int          height,
                                                          int          depth,
                                                          ktx_uint32_t face_lod_size,
                                                          void * /*pixels*/,
                                                          void *user_data)
{
	// Get mipmaps
	auto &mipmaps = *reinterpret_cast<std::vector<Mipmap> *>(user_data);
	assert(static_cast<size_t>(mip_level) < mipmaps.size() && "Not enough space in the mipmap vector");

	auto &mipmap         = mipmaps.at(mip_level);
	mipmap.level         = mip_level;
	mipmap.extent.width  = width;
	mipmap.extent.height = height;
	mipmap.extent.depth  = depth;

	// Set offset for the next mip level
	auto next_mip_level = static_cast<size_t>(mip_level + 1);
	if (next_mip_level < mipmaps.size())
	{
		mipmaps.at(next_mip_level).offset = mipmap.offset + face_lod_size;
	}

	return KTX_SUCCESS;
}

Ktx::Ktx(const std::string &name, const std::vector<uint8_t> &data) :
    Image{name}
{
	auto data_buffer = reinterpret_cast<const ktx_uint8_t *>(data.data());
	auto data_size   = static_cast<ktx_size_t>(data.size());

	ktxTexture *texture;
	auto        load_ktx_result = ktxTexture_CreateFromMemory(data_buffer,
                                                       data_size,
                                                       KTX_TEXTURE_CREATE_NO_FLAGS,
                                                       &texture);
	if (load_ktx_result != KTX_SUCCESS)
	{
		throw std::runtime_error{"Error loading KTX texture: " + name};
	}

	if (texture->pData)
	{
		// Already loaded
		set_data(texture->pData, texture->dataSize);
	}
	else
	{
		// Load
		auto &mut_data = get_mut_data();
		auto  size     = ktxTexture_GetSize(texture);
		mut_data.resize(size);
		auto load_data_result = ktxTexture_LoadImageData(texture, mut_data.data(), size);
		if (load_data_result != KTX_SUCCESS)
		{
			throw std::runtime_error{"Error loading KTX image data: " + name};
		}
	}

	// Update width and height
	set_width(texture->baseWidth);
	set_height(texture->baseHeight);
	set_depth(texture->baseDepth);

	// Update format
	auto updated_format = vkGetFormatFromOpenGLInternalFormat(texture->glInternalformat);
	set_format(updated_format);

	// Update mip levels
	auto &mipmap_levels = get_mut_mipmaps();
	mipmap_levels.resize(texture->numLevels);
	auto result = ktxTexture_IterateLevels(texture, optimal_tiling_callback, &mipmap_levels);
	if (result != KTX_SUCCESS)
	{
		throw std::runtime_error("Error loading KTX texture");
	}

	ktxTexture_Destroy(texture);
}

}        // namespace sg
}        // namespace vkb
