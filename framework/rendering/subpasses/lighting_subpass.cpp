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

#include "lighting_subpass.h"

#include "buffer_pool.h"
#include "rendering/render_context.h"
#include "scene_graph/components/camera.h"
#include "scene_graph/scene.h"

namespace vkb
{
LightingSubpass::LightingSubpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, sg::Camera &cam, sg::Scene &scene_) :
    Subpass{render_context, std::move(vertex_shader), std::move(fragment_shader)},
    camera{cam},
    scene{scene_}
{
}

void LightingSubpass::prepare()
{
	add_definitions(lighting_variant, {"MAX_DEFERRED_LIGHT_COUNT " + std::to_string(MAX_DEFERRED_LIGHT_COUNT)});
	add_definitions(lighting_variant, light_type_definitions);
	// Build all shaders upfront
	auto &resource_cache = render_context.get_device().get_resource_cache();
	resource_cache.request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, get_vertex_shader(), lighting_variant);
	resource_cache.request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, get_fragment_shader(), lighting_variant);
}

void LightingSubpass::draw(CommandBuffer &command_buffer)
{
	auto light_buffer = allocate_lights<DeferredLights>(scene.get_components<sg::Light>(), MAX_DEFERRED_LIGHT_COUNT);
	command_buffer.bind_buffer(light_buffer.get_buffer(), light_buffer.get_offset(), light_buffer.get_size(), 0, 4, 0);

	// Get shaders from cache
	auto &resource_cache     = command_buffer.get_device().get_resource_cache();
	auto &vert_shader_module = resource_cache.request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, get_vertex_shader(), lighting_variant);
	auto &frag_shader_module = resource_cache.request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, get_fragment_shader(), lighting_variant);

	std::vector<ShaderModule *> shader_modules{&vert_shader_module, &frag_shader_module};

	// Create pipeline layout and bind it
	auto &pipeline_layout = resource_cache.request_pipeline_layout(shader_modules);
	command_buffer.bind_pipeline_layout(pipeline_layout);

	// Get image views of the attachments
	auto &render_target = get_render_context().get_active_frame().get_render_target();
	auto &target_views  = render_target.get_views();

	// Bind depth, albedo, and normal as input attachments
	auto &depth_view = target_views.at(1);
	command_buffer.bind_input(depth_view, 0, 0, 0);

	auto &albedo_view = target_views.at(2);
	command_buffer.bind_input(albedo_view, 0, 1, 0);

	auto &normal_view = target_views.at(3);
	command_buffer.bind_input(normal_view, 0, 2, 0);

	// Set cull mode to front as full screen triangle is clock-wise
	RasterizationState rasterization_state;
	rasterization_state.cull_mode = VK_CULL_MODE_FRONT_BIT;
	command_buffer.set_rasterization_state(rasterization_state);

	// Populate uniform values
	LightUniform light_uniform;

	// Inverse resolution
	light_uniform.inv_resolution.x = 1.0f / render_target.get_extent().width;
	light_uniform.inv_resolution.y = 1.0f / render_target.get_extent().height;

	// Inverse view projection
	light_uniform.inv_view_proj = glm::inverse(vulkan_style_projection(camera.get_projection()) * camera.get_view());

	// Allocate a buffer using the buffer pool from the active frame to store uniform values and bind it
	auto &render_frame = get_render_context().get_active_frame();
	auto  allocation   = render_frame.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(LightUniform));
	allocation.update(light_uniform);
	command_buffer.bind_buffer(allocation.get_buffer(), allocation.get_offset(), allocation.get_size(), 0, 3, 0);

	// Draw full screen triangle triangle
	command_buffer.draw(3, 1, 0, 0);
}
}        // namespace vkb