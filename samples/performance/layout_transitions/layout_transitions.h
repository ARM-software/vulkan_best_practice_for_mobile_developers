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

#include "common/utils.h"
#include "rendering/render_pipeline.h"
#include "scene_graph/components/camera.h"
#include "vulkan_sample.h"

/**
 * @brief Transitioning images from UNDEFINED vs last known layout
 */
class LayoutTransitions : public vkb::VulkanSample
{
  public:
	LayoutTransitions();

	virtual ~LayoutTransitions() = default;

	virtual bool prepare(vkb::Platform &platform) override;

  private:
	enum LayoutTransitionType : int
	{
		UNDEFINED,
		LAST_LAYOUT
	};

	vkb::sg::Camera *camera{nullptr};

	vkb::RenderTarget create_render_target(vkb::core::Image &&swapchain_image);

	virtual void prepare_render_context() override;

	void draw(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target) override;

	virtual void draw_gui() override;

	VkImageLayout pick_old_layout(VkImageLayout last_layout);

	vkb::RenderPipeline gbuffer_pipeline;

	vkb::RenderPipeline lighting_pipeline;

	LayoutTransitionType layout_transition_type{LayoutTransitionType::UNDEFINED};
};

std::unique_ptr<vkb::VulkanSample> create_layout_transitions();
