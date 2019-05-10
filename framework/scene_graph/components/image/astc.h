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

#include "scene_graph/components/image.h"

#if defined(_WIN32) || defined(_WIN64)
// Windows.h defines IGNORE, so we must #undef it to avoid clashes with astc header
#	undef IGNORE
#endif
#include <astc_codec_internals.h>

namespace vkb
{
namespace sg
{
struct BlockDim
{
	uint8_t x;
	uint8_t y;
	uint8_t z;
};

class Astc : public Image
{
  public:
	/**
	 * @brief Decodes an ASTC image
	 * @param image Image to decode
	 */
	Astc(const Image &image);

	/**
	 * @brief Decodes ASTC data with an ASTC header
	 * @param name Name of the component
	 * @param data ASTC data with header
	 */
	Astc(const std::string &name, const std::vector<uint8_t> &data);

	virtual ~Astc() = default;

  private:
	/**
	 * @brief Decodes ASTC data
	 * @param blockdim Dimensions of the block
	 * @param extent Extent of the image
	 * @param data Pointer to ASTC image data
	 */
	void decode(BlockDim blockdim, VkExtent3D extent, const uint8_t *data);

	/**
	 * @brief Initializes ASTC library
	 */
	void init();
};
}        // namespace sg
}        // namespace vkb
