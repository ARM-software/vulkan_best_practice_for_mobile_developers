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

#include "node_animation.h"

#include "scene_graph/components/perspective_camera.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

namespace vkb
{
namespace sg
{
NodeAnimation::NodeAnimation(Node &node, TransformAnimFn animation_fn) :
    Script{node, ""},
    animation_fn{animation_fn}
{
}

void NodeAnimation::update(float delta_time)
{
	if (animation_fn)
	{
		animation_fn(get_node().get_component<Transform>(), delta_time);
	}
}

void NodeAnimation::set_animation(TransformAnimFn handle)
{
	animation_fn = handle;
}

void NodeAnimation::clear_animation()
{
	animation_fn = {};
}

}        // namespace sg
}        // namespace vkb
