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

#include <ctpl_stl.h>

#include "buffer_pool.h"
#include "common/utils.h"
#include "rendering/render_pipeline.h"
#include "rendering/subpasses/forward_subpass.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/perspective_camera.h"
#include "vulkan_sample.h"

/**
 * @brief Sample showing the use of secondary command buffers for
 *        multi-threaded recording, as well as the different
 *        strategies for recycling command buffers every frame
 */
class CommandBufferUsage : public vkb::VulkanSample
{
  public:
	CommandBufferUsage();

	virtual ~CommandBufferUsage() = default;

	virtual bool prepare(vkb::Platform &platform) override;

	virtual void update(float delta_time) override;

	/**
	 * @brief Helper structure used to set subpass state
	 */
	struct ForwardSubpassSecondaryState
	{
		uint32_t secondary_cmd_buf_count = 0;

		vkb::CommandBuffer::ResetMode command_buffer_reset_mode = vkb::CommandBuffer::ResetMode::ResetPool;

		bool multi_threading = false;

		uint32_t thread_count = 0;
	};

	/**
	 * @brief Overrides the draw method to allow for dividing draw calls
	 *        into multiple secondary command buffers, optionally
	 *        in different threads
	 */
	class ForwardSubpassSecondary : public vkb::ForwardSubpass
	{
	  public:
		ForwardSubpassSecondary(vkb::RenderContext &render_context,
		                        vkb::ShaderSource &&vertex_source, vkb::ShaderSource &&fragment_source,
		                        vkb::sg::Scene &scene, vkb::sg::Camera &camera);

		void draw(vkb::CommandBuffer &primary_command_buffer) override;

		void set_viewport(VkViewport &viewport);

		void set_scissor(VkRect2D &scissor);

		float get_avg_draws_per_buffer() const;

		ForwardSubpassSecondaryState &get_state();

	  private:
		/**
		 * @brief Records the necessary commands to draw the specified range of scene meshes
		 * @param command_buffer The primary command buffer to record
		 * @param nodes The meshes to draw
		 * @param mesh_start Index to the first mesh to draw
		 * @param mesh_end Index to the mesh where recording will stop (not included)
		 * @param thread_index Identifies the resources allocated for this thread
		 */
		void record_draw(vkb::CommandBuffer &command_buffer, const std::vector<std::pair<vkb::sg::Node *, vkb::sg::SubMesh *>> &nodes,
		                 uint32_t mesh_start, uint32_t mesh_end, size_t thread_index = 0);

		/**
		 * @brief Records the necessary commands to draw the specified range of scene meshes
		 *        The primary command buffer provided is used to initialize, record, end and return a
		 *        pointer to a new secondary command buffer.
		 * @param primary_command_buffer The primary command buffer used to inherit a secondary
		 * @param nodes The meshes to draw
		 * @param mesh_start Index to the first mesh to draw
		 * @param mesh_end Index to the mesh where recording will stop (not included)
		 * @param thread_index Identifies the resources allocated for this thread
		 * @return a pointer to the recorded secondary command buffer
		 */
		vkb::CommandBuffer *record_draw_secondary(vkb::CommandBuffer &primary_command_buffer, const std::vector<std::pair<vkb::sg::Node *, vkb::sg::SubMesh *>> &nodes,
		                                          uint32_t mesh_start, uint32_t mesh_end, size_t thread_index = 0);

		VkViewport viewport{};

		VkRect2D scissor{};

		vkb::ColorBlendAttachmentState color_blend_attachment{};

		vkb::ColorBlendState color_blend_state{};

		ForwardSubpassSecondaryState state{};

		float avg_draws_per_buffer{0};

		ctpl::thread_pool thread_pool;

		vkb::BufferAllocation light_buffer;
	};

  private:
	virtual void prepare_render_context() override;

	vkb::sg::PerspectiveCamera *camera{nullptr};

	void render(vkb::CommandBuffer &command_buffer) override;

	void draw_renderpass(vkb::CommandBuffer &primary_command_buffer, vkb::RenderTarget &render_target) override;

	void draw_gui() override;

	int gui_secondary_cmd_buf_count{0};

	uint32_t max_secondary_command_buffer_count{100};

	bool use_secondary_command_buffers{false};

	int gui_command_buffer_reset_mode{0};

	bool gui_multi_threading{false};

	const uint32_t MIN_THREAD_COUNT{4};

	uint32_t max_thread_count{0};
};

std::unique_ptr<vkb::VulkanSample> create_command_buffer_usage();
