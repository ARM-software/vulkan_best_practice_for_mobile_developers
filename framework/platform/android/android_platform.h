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

#include <android_native_app_glue.h>

#include "platform/platform.h"

namespace vkb
{
class AndroidPlatform : public Platform
{
  public:
	AndroidPlatform(android_app *app);

	virtual ~AndroidPlatform() = default;

	virtual bool initialize(std::unique_ptr<Application> &&app) override;

	virtual void create_window() override;

	virtual void main_loop() override;

	virtual void terminate(ExitCode code) override;

	virtual const char *get_surface_extension() override;

	/**
	 * @brief Sends a notification in the task bar
	 * @param message The message to display
	 */
	void send_notification(const std::string &message);

	ANativeActivity *get_activity();

	android_app *get_android_app();

  private:
	android_app *app{nullptr};

	std::string log_output;

	virtual std::vector<spdlog::sink_ptr> get_platform_sinks() override;
};
}        // namespace vkb
