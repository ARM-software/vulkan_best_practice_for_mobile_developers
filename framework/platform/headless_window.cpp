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

#include "headless_window.h"

namespace vkb
{
HeadlessWindow::HeadlessWindow(Platform &platform, uint32_t width, uint32_t height) :
    Window(platform, width, height)
{
}

VkSurfaceKHR HeadlessWindow::create_surface(VkInstance instance)
{
	return VK_NULL_HANDLE;
}

bool HeadlessWindow::should_close()
{
	return closed;
}

void HeadlessWindow::close()
{
	closed = true;
}

float HeadlessWindow::get_dpi_factor() const
{
	// This factor is used for scaling UI elements, so return 1.0f (1 x n = n)
	return 1.0f;
}
}        // namespace vkb
