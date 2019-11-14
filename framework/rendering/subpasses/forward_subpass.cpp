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

#include "rendering/subpasses/forward_subpass.h"

#include "common/utils.h"
#include "common/vk_common.h"
#include "rendering/render_context.h"
#include "scene_graph/components/camera.h"
#include "scene_graph/components/image.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/pbr_material.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/node.h"
#include "scene_graph/scene.h"

namespace vkb
{
ForwardSubpass::ForwardSubpass(RenderContext &render_context, ShaderSource &&vertex_source, ShaderSource &&fragment_source, sg::Scene &scene_, sg::Camera &camera) :
    GeometrySubpass{render_context, std::move(vertex_source), std::move(fragment_source), scene_, camera}
{
}

void ForwardSubpass::prepare()
{
	auto &device = render_context.get_device();
	for (auto &mesh : meshes)
	{
		for (auto &sub_mesh : mesh->get_submeshes())
		{
			auto &variant = sub_mesh->get_mut_shader_variant();

			// Same as Geometry except adds lighting definitions to sub mesh variants.
			add_definitions(variant, {"MAX_FORWARD_LIGHT_COUNT " + std::to_string(MAX_FORWARD_LIGHT_COUNT)});
			add_definitions(variant, light_type_definitions);

			auto &vert_module = device.get_resource_cache().request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, get_vertex_shader(), variant);
			auto &frag_module = device.get_resource_cache().request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, get_fragment_shader(), variant);

			vert_module.set_resource_dynamic("GlobalUniform");
			frag_module.set_resource_dynamic("GlobalUniform");
		}
	}
}

void ForwardSubpass::draw(CommandBuffer &command_buffer)
{
	auto lights_buffer = allocate_lights<ForwardLights>(scene.get_components<sg::Light>(), MAX_FORWARD_LIGHT_COUNT);
	command_buffer.bind_buffer(lights_buffer.get_buffer(), lights_buffer.get_offset(), lights_buffer.get_size(), 0, 4, 0);

	GeometrySubpass::draw(command_buffer);
}
}        // namespace vkb
