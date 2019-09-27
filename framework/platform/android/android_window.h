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

#include "common/vk_common.h"
#include "platform/window.h"

namespace vkb
{
/**
 * @brief Wrapper for a ANativeWindow, handles the window behaviour (including headless mode on Android)
 *        This class should not be responsible for destroying the underlying data it points to
 */
class AndroidWindow : public Window
{
  public:
	/**
	 * @brief Constructor
	 * @param platform The platform this window is created for
	 * @param window A reference to the location of the Android native window
	 * @param headless Whether the application is being rendered in headless mode
	 */
	AndroidWindow(Platform &platform, ANativeWindow *&window, bool headless = false);

	virtual ~AndroidWindow() = default;

	/**
	 * @brief Creates a Vulkan surface to the native window
	 *        If headless, this will return VK_NULL_HANDLE
	 */
	virtual VkSurfaceKHR create_surface(VkInstance instance) override;

	virtual bool should_close() override;

	virtual void close() override;

	virtual float get_dpi_factor() const override;

  private:
	// Handle to the android window
	ANativeWindow *&handle;

	// If true, return a VK_NULL_HANDLE on create_surface()
	bool headless;
};
}        // namespace vkb
