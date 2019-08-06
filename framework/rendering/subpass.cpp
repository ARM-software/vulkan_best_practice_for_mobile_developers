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

#include "subpass.h"

#include "render_context.h"

namespace vkb
{
glm::mat4 vulkan_style_projection(const glm::mat4 &proj)
{
	// Flip Y in clipspace. X = -1, Y = -1 is topLeft in Vulkan.
	glm::mat4 mat = proj;
	mat[1][1] *= -1;

	return mat;
}

Subpass::Subpass(RenderContext &render_context, ShaderSource &&vertex_source, ShaderSource &&fragment_source) :
    render_context{render_context},
    vertex_shader{std::move(vertex_source)},
    fragment_shader{std::move(fragment_source)}
{
}

void Subpass::update_render_target_attachments()
{
	auto &render_target = render_context.get_active_frame().get_render_target();

	render_target.set_input_attachments(input_attachments);
	render_target.set_output_attachments(output_attachments);
}

RenderContext &Subpass::get_render_context()
{
	return render_context;
}

const ShaderSource &Subpass::get_vertex_shader() const
{
	return vertex_shader;
}

const ShaderSource &Subpass::get_fragment_shader() const
{
	return fragment_shader;
}

DepthStencilState &Subpass::get_depth_stencil_state()
{
	return depth_stencil_state;
}

const std::vector<uint32_t> &Subpass::get_input_attachments() const
{
	return input_attachments;
}

void Subpass::set_input_attachments(std::vector<uint32_t> input)
{
	input_attachments = input;
}

const std::vector<uint32_t> &Subpass::get_output_attachments() const
{
	return output_attachments;
}

void Subpass::set_output_attachments(std::vector<uint32_t> output)
{
	output_attachments = output;
}
}        // namespace vkb
