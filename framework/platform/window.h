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

#include "common/vk_common.h"

namespace vkb
{
class Platform;

/**
 * @brief An interface class, declaring the behaviour of a Window
 */
class Window
{
  public:
	/**
	 * @brief Constructs a Window
	 * @param platform The platform this window is created for
	 * @param width The width of the window
	 * @param height The height of the window
	 */
	Window(Platform &platform, uint32_t width, uint32_t height);

	virtual ~Window() = default;

	/**
	 * @brief Gets a handle from the platform's Vulkan surface 
	 * @param instance The Vulkan instance
	 * @returns A VkSurfaceKHR handle, for use by the application
	 */
	virtual VkSurfaceKHR create_surface(VkInstance instance) = 0;

	/**
	 * @brief Checks if the window should be closed
	 */
	virtual bool should_close() = 0;

	/**
	 * @brief Handles the processing of all underlying window events
	 */
	virtual void process_events();

	/**
	 * @brief Requests to close the window
	 */
	virtual void close() = 0;

	/**
	 * @return The dot-per-inch scale factor
	 */
	virtual float get_dpi_factor() const = 0;

	Platform &get_platform();

	void resize(uint32_t width, uint32_t height);

	uint32_t get_width();

	uint32_t get_height();

  protected:
	Platform &platform;

  private:
	uint32_t width;

	uint32_t height;
};
}        // namespace vkb
