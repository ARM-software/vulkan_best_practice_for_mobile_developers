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

#include "utils.h"
#include "vulkan_sample.h"

#include "scene_graph/components/perspective_camera.h"

#include <iomanip>        // setprecision
#include <sstream>        // stringstream

class SurfaceRotationRenderContext : public vkb::RenderContext
{
  public:
	SurfaceRotationRenderContext(vkb::Device &device, std::unique_ptr<vkb::Swapchain> &&swapchain, bool pre_rotate);

	virtual ~SurfaceRotationRenderContext() = default;

	virtual void handle_surface_changes() override;

	inline void set_pre_rotate(bool pre_rotate)
	{
		this->pre_rotate = pre_rotate;
	}

	void recreate_swapchain();

  private:
	bool pre_rotate = false;
};

class SurfaceRotation : public vkb::VulkanSample
{
  public:
	SurfaceRotation();

	virtual ~SurfaceRotation() = default;

	virtual bool prepare(vkb::Platform &platform) override;

	virtual void update(float delta_time) override;

	static const char *transform_to_string(VkSurfaceTransformFlagBitsKHR flag);

  private:
	vkb::VertPushConstant vs_push_constant;

	vkb::FragPushConstant fs_push_constant;

	vkb::PipelineLayout *pipeline_layout{nullptr};

	vkb::sg::PerspectiveCamera* camera{nullptr};

	virtual void draw_gui() override;

	virtual void draw_scene(vkb::CommandBuffer &cmd_buf) override;

	void trigger_swapchain_recreation();

	bool pre_rotate = false;

	bool last_pre_rotate = false;
};

std::unique_ptr<vkb::VulkanSample> create_surface_rotation();
