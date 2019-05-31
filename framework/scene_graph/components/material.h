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
#include <unordered_map>
#include <vector>

#include "common.h"
#include "scene_graph/component.h"

namespace vkb
{
namespace sg
{
class Texture;

/**
 * @brief How the alpha value of the main factor and texture should be interpreted
 */
enum class AlphaMode
{
	/// Alpha value is ignored
	Opaque,
	/// Either full opaque or fully transparent
	Mask,
	/// Output is combined with the background
	Blend
};

class Material : public Component
{
  public:
	Material(const std::string &name);

	Material(Material &&other) = default;

	virtual ~Material() = default;

	virtual std::type_index get_type() override;

	std::unordered_map<std::string, Texture *> textures;

	/// Emissive color of the material
	glm::vec3 emissive{0.0f, 0.0f, 0.0f};

	/// Whether the material is double sided
	bool double_sided{false};

	/// Cutoff threshold when in Mask mode
	float alpha_cutoff{0.5f};

	/// Alpha rendering mode
	AlphaMode alpha_mode{AlphaMode::Opaque};
};

}        // namespace sg
}        // namespace vkb
