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

#include "rendering/render_pipeline.h"
#include "scene_graph/components/camera.h"
#include "scene_graph/components/perspective_camera.h"
#include "vulkan_sample.h"

class WaitIdle : public vkb::VulkanSample
{
  public:
	WaitIdle();

	virtual ~WaitIdle() = default;

	virtual bool prepare(vkb::Platform &platform) override;

	/**
	 * @brief This RenderContext is responsible containing the scene's RenderFrames
	 *		  It implements a custom wait_frame function which alternates between waiting with WaitIdle or Fences
	 */
	class CustomRenderContext : public vkb::RenderContext
	{
	  public:
		CustomRenderContext(vkb::Device &device, VkSurfaceKHR surface, uint32_t window_width, uint32_t window_height, int &wait_idle_enabled);

		virtual void wait_frame() override;

	  private:
		int &wait_idle_enabled;
	};

  private:
	vkb::sg::PerspectiveCamera *camera{nullptr};

	virtual void draw_gui() override;

	int wait_idle_enabled{0};
};

std::unique_ptr<vkb::VulkanSample> create_wait_idle();
