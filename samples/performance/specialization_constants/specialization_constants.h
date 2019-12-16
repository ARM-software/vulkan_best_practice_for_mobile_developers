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

#include "buffer_pool.h"
#include "rendering/render_pipeline.h"
#include "rendering/subpasses/forward_subpass.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/perspective_camera.h"
#include "vulkan_sample.h"

#define LIGHT_COUNT 1

struct alignas(16) CustomForwardLights
{
	uint32_t   count;
	vkb::Light lights[LIGHT_COUNT];
};

/**
 * @brief Using specialization constants
 */
class SpecializationConstants : public vkb::VulkanSample
{
  public:
	SpecializationConstants();

	virtual ~SpecializationConstants() = default;

	virtual bool prepare(vkb::Platform &platform) override;

	/**
	 * @brief This subpass is responsible for rendering a Scene
	 *		  It implements a custom draw function which passes a custom light count
	 */
	class ForwardSubpassCustomLights : public vkb::ForwardSubpass
	{
	  public:
		ForwardSubpassCustomLights(vkb::RenderContext &render_context,
		                           vkb::ShaderSource &&vertex_source, vkb::ShaderSource &&fragment_source,
		                           vkb::sg::Scene &scene, vkb::sg::Camera &camera);

		virtual void prepare() override;

		virtual void draw(vkb::CommandBuffer &command_buffer) override;
	};

  private:
	vkb::sg::PerspectiveCamera *camera{nullptr};

	virtual void draw_gui() override;

	virtual void render(vkb::CommandBuffer &command_buffer) override;

	std::unique_ptr<vkb::RenderPipeline> create_specialization_renderpass();

	std::unique_ptr<vkb::RenderPipeline> create_standard_renderpass();

	std::unique_ptr<vkb::RenderPipeline> specialization_constants_pipeline{};

	std::unique_ptr<vkb::RenderPipeline> standard_pipeline{};

	int specialization_constants_enabled{0};
};

std::unique_ptr<vkb::VulkanSample> create_specialization_constants();
