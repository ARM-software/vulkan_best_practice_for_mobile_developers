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

#include "common/error.h"

#include "buffer_pool.h"
#include "rendering/subpasses/geometry_subpass.h"

#define MAX_FORWARD_LIGHT_COUNT 16

namespace vkb
{
namespace sg
{
class Scene;
class Node;
class Mesh;
class SubMesh;
class Camera;
}        // namespace sg

struct alignas(16) ForwardLights
{
	uint32_t count;
	Light    lights[MAX_FORWARD_LIGHT_COUNT];
};

/**
 * @brief This subpass is responsible for rendering a Scene
 */
class ForwardSubpass : public GeometrySubpass
{
  public:
	/**
	 * @brief Constructs a subpass designed for forward rendering
	 * @param render_context Render context
	 * @param vertex_shader Vertex shader source
	 * @param fragment_shader Fragment shader source
	 * @param scene Scene to render on this subpass
	 * @param camera Camera used to look at the scene
	 */
	ForwardSubpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, sg::Scene &scene, sg::Camera &camera);

	virtual ~ForwardSubpass() = default;

	virtual void prepare() override;

	/**
	 * @brief Record draw commands
	 */
	virtual void draw(CommandBuffer &command_buffer) override;
};

}        // namespace vkb
