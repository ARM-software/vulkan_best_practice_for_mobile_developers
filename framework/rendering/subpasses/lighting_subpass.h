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
#include "rendering/subpass.h"

VKBP_DISABLE_WARNINGS()
#include "common/glm_common.h"
VKBP_ENABLE_WARNINGS()

#define MAX_DEFERRED_LIGHT_COUNT 100

namespace vkb
{
namespace sg
{
class Camera;
class Light;
class Scene;
}        // namespace sg

/**
 * @brief Light uniform structure for lighting shader
 * Inverse view projection matrix and inverse resolution vector are used
 * in lighting pass to reconstruct position from depth and frag coord
 */
struct alignas(16) LightUniform
{
	glm::mat4 inv_view_proj;
	glm::vec2 inv_resolution;
};

struct alignas(16) DeferredLights
{
	uint32_t count;
	Light    lights[MAX_DEFERRED_LIGHT_COUNT];
};

/**
 * @brief Lighting pass of Deferred Rendering
 */
class LightingSubpass : public Subpass
{
  public:
	LightingSubpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, sg::Camera &camera, sg::Scene &scene);

	virtual void prepare() override;

	void draw(CommandBuffer &command_buffer) override;

  private:
	sg::Camera &camera;

	sg::Scene &scene;

	ShaderVariant lighting_variant;
};

}        // namespace vkb
