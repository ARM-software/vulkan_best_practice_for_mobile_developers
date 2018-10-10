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
#include "scene_graph/script.h"

namespace vkb
{
namespace sg
{
class FreeCamera : public Script
{
  public:
	static const float TOUCH_DOWN_MOVE_FORWARD_WAIT_TIME;

	static const float ROTATION_MOVE_WEIGHT;

	static const float TRANSLATION_MOVE_WEIGHT;

	static const float TRANSLATION_MOVE_STEP;

	static const uint32_t TRANSLATION_MOVE_SPEED;

	FreeCamera(std::shared_ptr<Node> node);

	virtual ~FreeCamera() = default;

	virtual void update(float delta_time) override;

	virtual void input_event(const InputEvent &input_event) override;

	virtual void resize(uint32_t width, uint32_t height) override;

  private:
	float speed_multiplier{3.0f};

	glm::vec2 mouse_move_delta{0.0f};

	glm::vec2 mouse_last_pos{0.0f};

	glm::vec2 touch_move_delta{0.0f};

	glm::vec2 touch_last_pos{0.0f};

	float touch_pointer_time{0.0f};

	std::unordered_map<KeyCode, bool> key_pressed;

	std::unordered_map<MouseButton, bool> mouse_button_pressed;

	std::unordered_map<int32_t, bool> touch_pointer_pressed;
};
}        // namespace sg
}        // namespace vkb
