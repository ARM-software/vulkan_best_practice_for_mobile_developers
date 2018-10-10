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

#include "common.h"

namespace vkb
{
class Platform;

enum class EventSource
{
	Keyboard,
	Mouse,
	Touchscreen
};

class InputEvent
{
  public:
	InputEvent(Platform &platform, EventSource source);

	const Platform &get_platform() const;

	EventSource get_source() const;

  private:
	Platform &platform;

	EventSource source;
};

enum class KeyCode
{
	Unknown,
	Space,
	Apostrophe, /* ' */
	Comma,      /* , */
	Minus,      /* - */
	Period,     /* . */
	Slash,      /* / */
	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	Semicolon, /* ; */
	Equal,     /* = */
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	LeftBracket,  /* [ */
	Backslash,    /* \ */
	RightBracket, /* ] */
	GraveAccent,  /* ` */
	Escape,
	Enter,
	Tab,
	Backspace,
	Insert,
	DelKey,
	Right,
	Left,
	Down,
	Up,
	PageUp,
	PageDown,
	Home,
	End,
	Back,
	CapsLock,
	ScrollLock,
	NumLock,
	PrintScreen,
	Pause,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	KP_0,
	KP_1,
	KP_2,
	KP_3,
	KP_4,
	KP_5,
	KP_6,
	KP_7,
	KP_8,
	KP_9,
	KP_Decimal,
	KP_Divide,
	KP_Multiply,
	KP_Subtract,
	KP_Add,
	KP_Enter,
	KP_Equal,
	LeftShift,
	LeftControl,
	LeftAlt,
	RightShift,
	RightControl,
	RightAlt
};

enum class KeyAction
{
	Down,
	Up,
	Repeat,
	Unknown
};

class KeyInputEvent : public InputEvent
{
  public:
	KeyInputEvent(Platform &platform, KeyCode code, KeyAction action);

	KeyCode get_code() const;

	KeyAction get_action() const;

  private:
	KeyCode code;

	KeyAction action;
};

enum class MouseButton
{
	Left,
	Right,
	Middle,
	Back,
	Forward,
	Unknown
};

enum class MouseAction
{
	Down,
	Up,
	Move,
	Unknown
};

class MouseButtonInputEvent : public InputEvent
{
  public:
	MouseButtonInputEvent(Platform &platform, MouseButton button, MouseAction action, float pos_x, float pos_y);

	MouseButton get_button() const;

	MouseAction get_action() const;

	float get_pos_x() const;

	float get_pos_y() const;

  private:
	MouseButton button;

	MouseAction action;

	float pos_x;

	float pos_y;
};

enum class TouchAction
{
	Down,
	Up,
	Move,
	Cancel,
	Unknown
};

class TouchInputEvent : public InputEvent
{
  public:
	TouchInputEvent(Platform &platform, std::int32_t pointer_id, TouchAction action, float pos_x, float pos_y);

	TouchAction get_action() const;

	std::int32_t get_pointer_id() const;

	float get_pos_x() const;

	float get_pos_y() const;

  private:
	TouchAction action;

	std::int32_t pointer_id;

	float pos_x;

	float pos_y;
};
}        // namespace vkb
