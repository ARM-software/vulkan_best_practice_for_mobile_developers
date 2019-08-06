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
#include "rendering/subpasses/scene_subpass.h"
#include "scene_graph/components/camera.h"
#include "vulkan_sample.h"

/**
 * @brief Command buffer utilization
 */
class CommandBufferUsage : public vkb::VulkanSample
{
  public:
	CommandBufferUsage();

	virtual ~CommandBufferUsage() = default;

	virtual bool prepare(vkb::Platform &platform) override;

	virtual void update(float delta_time) override;

	class SceneSubpassSecondary : public vkb::SceneSubpass
	{
	  public:
		SceneSubpassSecondary(vkb::RenderContext &render_context, vkb::ShaderSource &&vertex_source, vkb::ShaderSource &&fragment_source, vkb::sg::Scene &scene, vkb::sg::Camera &camera);

		void draw(vkb::CommandBuffer &primary_command_buffer) override;

		void set_use_secondary_command_buffers(bool use_secondary);

		void set_command_buffer_reset_mode(vkb::CommandBuffer::ResetMode reset_mode);

		void set_viewport(VkViewport &viewport);

		void set_scissor(VkRect2D &scissor);

	  private:
		bool use_secondary_command_buffers{false};

		vkb::CommandBuffer::ResetMode command_buffer_reset_mode{vkb::CommandBuffer::ResetMode::ResetPool};

		VkViewport viewport{};

		VkRect2D scissor{};
	};

  private:
	SceneSubpassSecondary *scene_subpass_ptr{nullptr};

	vkb::sg::Camera *camera{nullptr};

	void render(vkb::CommandBuffer &command_buffer) override;

	void draw_swapchain_renderpass(vkb::CommandBuffer &primary_command_buffer, vkb::RenderTarget &render_target) override;

	void draw_gui() override;

	std::unique_ptr<vkb::CommandPool> command_pool{nullptr};

	bool use_secondary_command_buffers{false};

	int reuse_selection{0};
};

std::unique_ptr<vkb::VulkanSample> create_command_buffer_usage();
