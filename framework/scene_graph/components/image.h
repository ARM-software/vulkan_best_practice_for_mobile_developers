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

#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include <volk.h>

#include "core/image.h"
#include "core/image_view.h"
#include "scene_graph/component.h"

namespace vkb
{
namespace sg
{
/**
 * @param format Vulkan format
 * @return Whether the vulkan format is ASTC
 */
bool is_astc(VkFormat format);

/**
 * @brief Mipmap information
 */
struct Mipmap
{
	/// Mipmap level
	uint32_t level = 0;

	/// Byte offset used for uploading
	uint32_t offset = 0;

	/// Width depth and height of the mipmap
	VkExtent3D extent = {0, 0, 0};
};

class Image : public Component
{
  public:
	Image(const std::string &name, std::vector<uint8_t> &&data = {}, std::vector<Mipmap> &&mipmaps = {{}});

	static std::unique_ptr<Image> load(const std::string &name, const std::string &uri);

	virtual ~Image() = default;

	virtual std::type_index get_type() override;

	const std::vector<uint8_t> &get_data() const;

	void clear_data();

	VkFormat get_format() const;

	const VkExtent3D &get_extent() const;

	const std::vector<Mipmap> &get_mipmaps() const;

	void generate_mipmaps();

	void create_vk_image(Device &device);

	const core::Image &get_vk_image() const;

	const core::ImageView &get_vk_image_view() const;

  protected:
	std::vector<uint8_t> &get_mut_data();

	void set_data(const uint8_t *raw_data, size_t size);

	void set_format(VkFormat format);

	void set_width(uint32_t width);

	void set_height(uint32_t height);

	void set_depth(uint32_t depth);

	Mipmap &get_mipmap(size_t index);

	std::vector<Mipmap> &get_mut_mipmaps();

  private:
	std::vector<uint8_t> data;

	VkFormat format{VK_FORMAT_UNDEFINED};

	std::vector<Mipmap> mipmaps{{}};

	std::unique_ptr<core::Image> vk_image;

	std::unique_ptr<core::ImageView> vk_image_view;
};

}        // namespace sg
}        // namespace vkb
