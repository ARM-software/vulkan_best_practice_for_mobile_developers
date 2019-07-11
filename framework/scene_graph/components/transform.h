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

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
VKBP_ENABLE_WARNINGS()

#include "scene_graph/component.h"

namespace vkb
{
namespace sg
{
class Node;

class Transform : public Component
{
  public:
	Transform(Node &node);

	virtual ~Transform() = default;

	Node &get_node();

	virtual std::type_index get_type() override;

	void set_translation(const glm::vec3 &translation);

	void set_rotation(const glm::quat &rotation);

	void set_scale(const glm::vec3 &scale);

	const glm::vec3 &get_translation() const;

	const glm::quat &get_rotation() const;

	const glm::vec3 &get_scale() const;

	void set_matrix(const glm::mat4 &matrix);

	glm::mat4 get_matrix() const;

	glm::mat4 get_world_matrix();

	/**
	 * @brief Marks the world transform invalid if any of
	 *        the local transform are changed or the parent
	 *        world transform has changed.
	 */
	void invalidate_world_matrix();

  private:
	Node &node;

	glm::vec3 translation = glm::vec3(0.0, 0.0, 0.0);

	glm::quat rotation = glm::quat(1.0, 0.0, 0.0, 0.0);

	glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 world_matrix = glm::mat4(1.0);

	bool update_world_matrix = false;

	void update_world_transform();
};

}        // namespace sg
}        // namespace vkb
