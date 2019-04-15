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

#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include "common.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/script.h"

/**
 *	@param std::shared_ptr<vkb::sg::Transform> The transform to animate
 *  @param float The delta time of the frame to scale animations
 */
using TransformAnimFn = std::function<void(std::shared_ptr<vkb::sg::Transform>, float)>;

namespace vkb
{
namespace sg
{
class NodeAnimation : public Script
{
  public:
	NodeAnimation(std::shared_ptr<Node> node, TransformAnimFn animation_fn);

	virtual ~NodeAnimation() = default;

	virtual void update(float delta_time) override;

	void set_animation(TransformAnimFn handle);

	void clear_animation();

  private:
	TransformAnimFn animation_fn{};
};
}        // namespace sg
}        // namespace vkb
