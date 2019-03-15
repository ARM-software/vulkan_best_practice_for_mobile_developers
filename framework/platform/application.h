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

#include <string>

#include "platform/configuration.h"
#include "platform/input_events.h"

namespace vkb
{
class Platform;

class Application
{
  public:
	Application();

	virtual ~Application() = default;

	virtual bool prepare(Platform &platform);

	virtual void update(float delta_time) = 0;

	virtual void finish();

	/// @brief Handles resizing of the window
	/// @param width New width of the window
	/// @param height New height of the window
	virtual void resize(const uint32_t width, const uint32_t height);

	/// @brief Handles input events of the window
	/// @param input_event The input event object
	virtual void input_event(const InputEvent &input_event);

	/// @brief Returns a reference to the name of the app
	/// @returns A string reference
	const std::string &get_name() const;

	void set_name(const std::string &name);

	Configuration &get_configuration();

	/**
	 * @brief Change focus state
	 * @praram flag Current focus state of window
	 */
	void set_focus(bool flag);

	/**
	 * @return True if window is in focus, false otherwise
	 */
	bool is_focused() const;

  private:
	/// The name of the app
	std::string name{};

	/// The configurations of the sample
	Configuration configuration{};

	/**
	 * @brief Focus state of application
	 */
	bool focus{true};
};
}        // namespace vkb
