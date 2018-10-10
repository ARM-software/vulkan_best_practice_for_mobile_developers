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

#include "texture.h"

namespace vkb
{
namespace sg
{
Texture::Texture(const std::string &name) :
    Component{name}
{}

std::type_index Texture::get_type()
{
	return typeid(Texture);
}
void Texture::set_image(std::shared_ptr<Image> image)
{
	if (image)
	{
		this->image = image;
	}
}
std::shared_ptr<Image> Texture::get_image()
{
	return image;
}
void Texture::set_sampler(std::shared_ptr<Sampler> sampler)
{
	if (sampler)
	{
		this->sampler = sampler;
	}
}
std::shared_ptr<Sampler> Texture::get_sampler()
{
	return sampler;
}
}        // namespace sg
}        // namespace vkb
