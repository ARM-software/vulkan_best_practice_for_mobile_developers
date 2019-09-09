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

#include "android_window.h"

#include "platform/android/android_platform.h"

namespace vkb
{
AndroidWindow::AndroidWindow(Platform &platform, ANativeWindow *&window, bool headless) :
    Window(platform, 0, 0),
    handle{window},
    headless{headless}
{
}

VkSurfaceKHR AndroidWindow::create_surface(VkInstance instance)
{
	if (instance == VK_NULL_HANDLE || !handle || headless)
	{
		return VK_NULL_HANDLE;
	}

	VkSurfaceKHR surface{};

	VkAndroidSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};

	info.window = handle;

	VK_CHECK(vkCreateAndroidSurfaceKHR(instance, &info, nullptr, &surface));

	return surface;
}

bool AndroidWindow::should_close()
{
	return handle == nullptr;
}

void AndroidWindow::close()
{
	auto &android_platform = dynamic_cast<AndroidPlatform &>(platform);
	ANativeActivity_finish(android_platform.get_android_app()->activity);
}

float AndroidWindow::get_dpi_factor() const
{
	auto &android_platform = dynamic_cast<AndroidPlatform &>(platform);
	return AConfiguration_getDensity(android_platform.get_android_app()->config) / static_cast<float>(ACONFIGURATION_DENSITY_MEDIUM);
}
}        // namespace vkb
