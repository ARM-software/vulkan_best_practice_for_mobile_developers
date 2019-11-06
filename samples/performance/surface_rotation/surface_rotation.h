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

#include <iomanip>        // setprecision
#include <sstream>        // stringstream

#include "common/vk_common.h"
#include "rendering/render_pipeline.h"
#include "scene_graph/components/perspective_camera.h"
#include "vulkan_sample.h"

/**
 * @brief Appropriate use of surface rotation
 */
class SurfaceRotation : public vkb::VulkanSample
{
  public:
	SurfaceRotation();

	virtual ~SurfaceRotation() = default;

	virtual bool prepare(vkb::Platform &platform) override;

	virtual void update(float delta_time) override;

	static const char *transform_to_string(VkSurfaceTransformFlagBitsKHR flag);

  private:
	vkb::sg::PerspectiveCamera *camera{nullptr};

	virtual void draw_gui() override;

	void recreate_swapchain();

	bool pre_rotate = false;

	bool last_pre_rotate = false;

	/*
	* @brief Returns the preTransform value to use when recreating
	*        the swapchain, which depends on whether or not the
	*        application is implementing pre-rotation
	*/
	VkSurfaceTransformFlagBitsKHR select_pre_transform();

	/* @brief 180 degree rotations do not trigger a resize, but
	 *        if pre_rotation is enabled a new swapchain
	 *        needs to be created with the corresponding
	 *        preTransform value
	 */
	void handle_no_resize_rotations();
};

std::unique_ptr<vkb::VulkanSample> create_surface_rotation();
