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

#include "core/buffer.h"

#include "render_frame.h"

#include "scene_graph/components/camera.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/scene.h"

#include "utils.h"

namespace vkb
{
/**
 * @brief Global uniform structure for base shader
 */
struct alignas(16) GlobalUniform
{
	glm::mat4 model;

	glm::mat4 camera_view_proj;

	glm::vec4 light_pos;

	glm::vec4 light_color;
};

/**
 * @brief PBR material uniform for base shader
 */
struct PBRMaterialUniform
{
	glm::vec4 base_color_factor;

	float metallic_factor;

	float roughness_factor;
};

class RenderPipeline : public NonCopyable
{
  public:
	RenderPipeline(RenderContext &render_context, sg::Scene &scene, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader);

	void draw_scene(CommandBuffer &command_buffer, sg::Camera &camera);

  protected:
	virtual void draw_scene_submesh(CommandBuffer &command_buffer, sg::SubMesh &sub_mesh);

  private:
	RenderContext &render_context;

	std::vector<sg::Mesh *> meshes;

	ShaderSource vertex_shader;

	ShaderSource fragment_shader;

	GlobalUniform global_uniform;
};

}        // namespace vkb
