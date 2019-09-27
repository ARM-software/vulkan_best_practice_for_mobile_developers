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

#include "platform/window.h"

namespace vkb
{
/**
 * @brief Surface-less implementation of a Window, for use in headless rendering
 */
class HeadlessWindow : public Window
{
  public:
	HeadlessWindow(Platform &platform, uint32_t width = 1028, uint32_t height = 720);

	virtual ~HeadlessWindow() = default;

	/**
	 * @brief A direct window doesn't have a surface
	 * @returns VK_NULL_HANDLE
	 */
	virtual VkSurfaceKHR create_surface(VkInstance instance) override;

	virtual bool should_close() override;

	virtual void close() override;

	virtual float get_dpi_factor() const override;

  private:
	bool closed{false};
};
}        // namespace vkb
