/* Copyright (c) 2018-2019, Arm Limited and Contributors
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
#include "vulkan_sample.h"

#include "scene_graph/components/perspective_camera.h"

#include <iomanip>        // setprecision
#include <sstream>        // stringstream

class RenderPassesSample : public vkb::VulkanSample
{
  public:
	RenderPassesSample();

	bool prepare(vkb::Platform &platform) override;

	virtual void update(float delta_time) override;

	void draw_gui() override;

	/** 
	 * @brief Struct that contains radio button labeling and the value 
	 *			which one is selected
	 */
	struct RadioButtonGroup
	{
		const char *description;

		std::vector<const char *> options;

		int value;
	};

  private:
	void reset_stats_view() override;
	void draw_swapchain_renderpass(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target) override;
	void draw_scene(vkb::CommandBuffer &command_buffer) override;

	std::unique_ptr<vkb::RenderPipeline> render_pipeline{nullptr};

	vkb::sg::PerspectiveCamera *camera{nullptr};

	RadioButtonGroup load{
	    "Color attachment load operation",
	    {"Load", "Clear", "Don't care"},
	    0};

	RadioButtonGroup store{
	    "Depth attachment store operation",
	    {"Store", "Don't care"},
	    0};

	std::vector<RadioButtonGroup *> radio_buttons = {&load, &store};

	float frame_rate;
};

std::unique_ptr<vkb::VulkanSample> create_render_passes();
