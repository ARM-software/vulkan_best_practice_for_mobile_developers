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

#include "rendering/subpass.h"

namespace vkb
{
namespace sg
{
class Scene;
class Mesh;
class SubMesh;
class Camera;
}        // namespace sg

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

/**
 * @brief This subpass is responsible for rendering a Scene
 */
class SceneSubpass : public Subpass
{
  public:
	/**
	 * @brief Constructs a subpass
	 * @param render_context Render context
	 * @param vertex_shader Vertex shader source
	 * @param fragment_shader Fragment shader source
	 * @param scene Scene to render on this subpass
	 * @param camera Camera used to look at the scene
	 */
	SceneSubpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, sg::Scene &scene, sg::Camera &camera);

	virtual ~SceneSubpass() = default;

	/**
	 * @brief record draw commands
	 */
	void draw(CommandBuffer &command_buffer) override;

  private:
	void draw_submesh(CommandBuffer &command_buffer, sg::SubMesh &sub_mesh);

	std::vector<sg::Mesh *> meshes;

	sg::Camera &camera;

	GlobalUniform global_uniform;
};

}        // namespace vkb
