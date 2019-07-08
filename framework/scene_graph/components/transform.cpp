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

#include "transform.h"

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
VKBP_ENABLE_WARNINGS()

#include "scene_graph/node.h"

namespace vkb
{
namespace sg
{
Transform::Transform(Node &n) :
    node{n}
{
}

Node &Transform::get_node()
{
	return node;
}

std::type_index Transform::get_type()
{
	return typeid(Transform);
}

void Transform::set_translation(const glm::vec3 &new_translation)
{
	translation = new_translation;

	invalidate_world_matrix();
}

void Transform::set_rotation(const glm::quat &new_rotation)
{
	rotation = new_rotation;

	invalidate_world_matrix();
}

void Transform::set_scale(const glm::vec3 &new_scale)
{
	scale = new_scale;

	invalidate_world_matrix();
}

const glm::vec3 &Transform::get_translation() const
{
	return translation;
}

const glm::quat &Transform::get_rotation() const
{
	return rotation;
}

const glm::vec3 &Transform::get_scale() const
{
	return scale;
}

void Transform::set_matrix(const glm::mat4 &matrix)
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, rotation, translation, skew, perspective);

	invalidate_world_matrix();
}

glm::mat4 Transform::get_matrix() const
{
	return glm::translate(glm::mat4(1.0), translation) *
	       glm::mat4_cast(rotation) *
	       glm::scale(glm::mat4(1.0), scale);
}

glm::mat4 Transform::get_world_matrix()
{
	update_world_transform();

	return world_matrix;
}

void Transform::invalidate_world_matrix()
{
	update_world_matrix = true;
}

void Transform::update_world_transform()
{
	if (!update_world_matrix)
	{
		return;
	}

	world_matrix = get_matrix();

	auto parent = node.get_parent();

	if (parent)
	{
		auto &transform = parent->get_component<Transform>();
		world_matrix    = world_matrix * transform.get_world_matrix();
	}

	update_world_matrix = false;
}

}        // namespace sg
}        // namespace vkb
