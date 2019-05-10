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

#include "scene_graph/components/image/stb.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace vkb
{
namespace sg
{
Stb::Stb(const std::string &name, const std::vector<uint8_t> &data) :
    Image{name}
{
	int width;
	int height;
	int comp;
	int req_comp = 4;

	auto data_buffer = reinterpret_cast<const stbi_uc *>(data.data());
	auto data_size   = static_cast<int>(data.size());

	auto raw_data = stbi_load_from_memory(data_buffer, data_size, &width, &height, &comp, req_comp);

	if (!raw_data)
	{
		throw std::runtime_error{"Failed to load " + name + ": " + stbi_failure_reason()};
	}

	set_data(raw_data, width * height * req_comp);
	stbi_image_free(raw_data);

	set_format(VK_FORMAT_R8G8B8A8_UNORM);
	set_width(to_u32(width));
	set_height(to_u32(height));
	set_depth(1u);
}

}        // namespace sg
}        // namespace vkb
