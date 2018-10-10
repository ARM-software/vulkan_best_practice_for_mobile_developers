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

#include "input_events.h"

namespace vkb
{
InputEvent::InputEvent(Platform &platform, EventSource source) :
    platform{platform},
    source{source}
{
}

const Platform &InputEvent::get_platform() const
{
	return platform;
}

EventSource InputEvent::get_source() const
{
	return source;
}

KeyInputEvent::KeyInputEvent(Platform &platform, KeyCode code, KeyAction action) :
    InputEvent{platform, EventSource::Keyboard},
    code{code},
    action{action}
{
}

KeyCode KeyInputEvent::get_code() const
{
	return code;
}

KeyAction KeyInputEvent::get_action() const
{
	return action;
}

MouseButtonInputEvent::MouseButtonInputEvent(Platform &platform, MouseButton button, MouseAction action, float pos_x, float pos_y) :
    InputEvent{platform, EventSource::Mouse},
    button{button},
    action{action},
    pos_x{pos_x},
    pos_y{pos_y}
{
}

MouseButton MouseButtonInputEvent::get_button() const
{
	return button;
}

MouseAction MouseButtonInputEvent::get_action() const
{
	return action;
}

float MouseButtonInputEvent::get_pos_x() const
{
	return pos_x;
}

float MouseButtonInputEvent::get_pos_y() const
{
	return pos_y;
}

TouchInputEvent::TouchInputEvent(Platform &platform, std::int32_t pointer_id, TouchAction action, float pos_x, float pos_y) :
    InputEvent{platform, EventSource::Touchscreen},
    action{action},
    pointer_id{pointer_id},
    pos_x{pos_x},
    pos_y{pos_y}
{
}

TouchAction TouchInputEvent::get_action() const
{
	return action;
}

std::int32_t TouchInputEvent::get_pointer_id() const
{
	return pointer_id;
}

float TouchInputEvent::get_pos_x() const
{
	return pos_x;
}

float TouchInputEvent::get_pos_y() const
{
	return pos_y;
}
}        // namespace vkb
