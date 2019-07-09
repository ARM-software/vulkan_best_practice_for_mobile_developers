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

#include "common/helpers.h"
#include "core/command_buffer.h"
#include "core/shader_module.h"

namespace vkb
{
class RenderContext;

/**
 * @brief Calculates the vulkan style projection matrix
 * @param proj The projection matrix
 * @return The vulkan style projection matrix
 */
glm::mat4 vulkan_style_projection(const glm::mat4 &proj);

/**
 * @brief This class defines an interface for subpasses
 *        where they need to implement the draw function.
 *        It is used to construct a RenderPipeline
 */
class Subpass : public NonCopyable
{
  public:
	Subpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader);

	virtual ~Subpass() = default;

	/**
	 * @brief Updates the render target attachments with the ones stored in this subpass
	 *        This function is called by the RenderPipeline before beginning the render
	 *        pass and before proceeding with a new subpass.
	 */
	void update_render_target_attachments();

	/**
	 * @brief Draw virtual function
	 * @param command_buffer Command buffer to use to record draw commands
	 */
	virtual void draw(CommandBuffer &command_buffer) = 0;

	RenderContext &get_render_context();

	const ShaderSource &get_vertex_shader() const;

	const ShaderSource &get_fragment_shader() const;

	DepthStencilState &get_depth_stencil_state();

	const std::vector<uint32_t> &get_input_attachments() const;

	void set_input_attachments(std::vector<uint32_t> input);

	const std::vector<uint32_t> &get_output_attachments() const;

	void set_output_attachments(std::vector<uint32_t> output);

  private:
	RenderContext &render_context;

	ShaderSource vertex_shader;

	ShaderSource fragment_shader;

	DepthStencilState depth_stencil_state{};

	/// Default to no input attachments
	std::vector<uint32_t> input_attachments = {};

	/// Default to swapchain output attachment
	std::vector<uint32_t> output_attachments = {0};
};

}        // namespace vkb
