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

#include "camera.h"

#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"

namespace vkb
{
namespace sg
{
Camera::Camera(const std::string &name) :
    Component{name}
{}

std::type_index Camera::get_type()
{
	return typeid(Camera);
}

glm::mat4 Camera::get_view()
{
	if (!node)
	{
		throw std::runtime_error{"Camera component is not attached to a node"};
	}

	auto &transform = node->get_component<Transform>();
	return pre_rotation * glm::inverse(transform.get_world_matrix());
}

void Camera::set_node(Node &n)
{
	node = &n;
}

Node *Camera::get_node()
{
	return node;
}

void Camera::set_pre_rotation(const glm::mat4 &pr)
{
	pre_rotation = pr;
}
}        // namespace sg
}        // namespace vkb
