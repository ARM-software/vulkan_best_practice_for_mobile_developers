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

#include "render_pipeline.h"

#include "scene_graph/components/camera.h"
#include "scene_graph/components/image.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/pbr_material.h"
#include "scene_graph/components/sampler.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/node.h"

namespace vkb
{
RenderPipeline::RenderPipeline(RenderContext &render_context, sg::Scene &scene, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader) :
    render_context{render_context},
    meshes{scene.get_components<sg::Mesh>()},
    vertex_shader{std::move(vertex_shader)},
    fragment_shader{std::move(fragment_shader)}
{
	global_uniform.light_pos   = glm::vec4(500.0f, 1550.0f, 0.0f, 1.0);
	global_uniform.light_color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	Device &device = render_context.get_device();

	// Build all shader variance upfront
	for (auto &mesh : meshes)
	{
		for (auto &sub_mesh : mesh->get_submeshes())
		{
			ShaderModule &vert_shader_module = device.get_resource_cache().request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, this->vertex_shader, sub_mesh->get_shader_variant());
			ShaderModule &frag_shader_module = device.get_resource_cache().request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, this->fragment_shader, sub_mesh->get_shader_variant());

			vert_shader_module.set_resource_dynamic("GlobalUniform");
			frag_shader_module.set_resource_dynamic("GlobalUniform");
		}
	}
}

void RenderPipeline::draw_scene(CommandBuffer &command_buffer, sg::Camera &camera)
{
	std::multimap<float, std::pair<sg::Node *, sg::SubMesh *>> opaque_nodes;
	std::multimap<float, std::pair<sg::Node *, sg::SubMesh *>> transparent_nodes;

	glm::mat4 camera_transform = camera.get_node()->get_transform().get_world_matrix();

	// Sort objects based on distance from camera and type
	for (auto &mesh : meshes)
	{
		for (auto &node : mesh->get_nodes())
		{
			glm::mat4 node_transform = node->get_transform().get_world_matrix();

			const sg::AABB &mesh_bounds = mesh->get_bounds();

			sg::AABB world_bounds{mesh_bounds.get_min(), mesh_bounds.get_max()};
			world_bounds.transform(node_transform);

			float distance = glm::length(glm::vec3(camera_transform[3]) - world_bounds.get_center());

			for (auto &sub_mesh : mesh->get_submeshes())
			{
				if (sub_mesh->get_material()->alpha_mode == sg::AlphaMode::Blend)
				{
					transparent_nodes.emplace(distance, std::make_pair(node, sub_mesh));
				}
				else
				{
					opaque_nodes.emplace(distance, std::make_pair(node, sub_mesh));
				}
			}
		}
	}

	global_uniform.camera_view_proj = vkb::vulkan_style_projection(camera.get_projection()) * camera.get_view();

	auto &render_frame = render_context.get_active_frame();

	// Draw opaque objects in front-to-back order
	for (auto node_it = opaque_nodes.begin(); node_it != opaque_nodes.end(); node_it++)
	{
		auto &transform = node_it->second.first->get_transform();

		auto allocation = render_frame.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniform));

		global_uniform.model = transform.get_world_matrix();

		allocation.update(0, global_uniform);

		command_buffer.bind_buffer(allocation.get_buffer(), allocation.get_offset(), allocation.get_size(), 0, 1, 0);

		draw_scene_submesh(command_buffer, *node_it->second.second);
	}

	// Enable alpha blending
	ColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.blend_enable           = VK_TRUE;
	color_blend_attachment.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.src_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	ColorBlendState color_blend_state{};
	color_blend_state.attachments = {color_blend_attachment};

	command_buffer.set_color_blend_state(color_blend_state);

	DepthStencilState depth_stencil_state{};
	depth_stencil_state.depth_write_enable = VK_FALSE;

	command_buffer.set_depth_stencil_state(depth_stencil_state);

	// Draw transparent objects in back-to-front order
	for (auto node_it = transparent_nodes.rbegin(); node_it != transparent_nodes.rend(); node_it++)
	{
		auto &transform = node_it->second.first->get_transform();

		auto allocation = render_frame.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniform));

		global_uniform.model = transform.get_world_matrix();

		allocation.update(0, global_uniform);

		command_buffer.bind_buffer(allocation.get_buffer(), allocation.get_offset(), allocation.get_size(), 0, 1, 0);

		draw_scene_submesh(command_buffer, *node_it->second.second);
	}
}

void RenderPipeline::draw_scene_submesh(CommandBuffer &command_buffer, sg::SubMesh &sub_mesh)
{
	Device &device = command_buffer.get_device();

	RasterizationState rasterization_state{};

	if (sub_mesh.get_material()->double_sided)
	{
		rasterization_state.cull_mode = VK_CULL_MODE_NONE;
	}

	command_buffer.set_rasterization_state(rasterization_state);

	ShaderModule &vert_shader_module = device.get_resource_cache().request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader, sub_mesh.get_shader_variant());
	ShaderModule &frag_shader_module = device.get_resource_cache().request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader, sub_mesh.get_shader_variant());

	std::vector<ShaderModule *> shader_modules{&vert_shader_module, &frag_shader_module};

	PipelineLayout &pipeline_layout = device.get_resource_cache().request_pipeline_layout(shader_modules);

	command_buffer.bind_pipeline_layout(pipeline_layout);

	auto pbr_material = dynamic_cast<const sg::PBRMaterial *>(sub_mesh.get_material());

	PBRMaterialUniform pbr_material_uniform{};
	pbr_material_uniform.base_color_factor = pbr_material->base_color_factor;
	pbr_material_uniform.metallic_factor   = pbr_material->metallic_factor;
	pbr_material_uniform.roughness_factor  = pbr_material->roughness_factor;

	command_buffer.push_constants(0, pbr_material_uniform);

	DescriptorSetLayout &descriptor_set_layout = pipeline_layout.get_set_layout(0);

	for (auto &texture : sub_mesh.get_material()->textures)
	{
		VkDescriptorSetLayoutBinding layout_binding;

		if (descriptor_set_layout.has_layout_binding(texture.first, layout_binding))
		{
			command_buffer.bind_image(texture.second->get_image()->get_vk_image_view(),
			                          texture.second->get_sampler()->vk_sampler,
			                          0, layout_binding.binding, 0);
		}
	}

	auto vertex_input_resources = pipeline_layout.get_vertex_input_attributes();

	VertexInputState vertex_input_state;

	for (auto &input_resource : vertex_input_resources)
	{
		sg::VertexAttribute attribute;

		if (!sub_mesh.get_attribute(input_resource.name, attribute))
		{
			continue;
		}

		VkVertexInputAttributeDescription vertex_attribute{};
		vertex_attribute.binding  = input_resource.location;
		vertex_attribute.format   = attribute.format;
		vertex_attribute.location = input_resource.location;
		vertex_attribute.offset   = attribute.offset;

		vertex_input_state.attributes.push_back(vertex_attribute);

		VkVertexInputBindingDescription vertex_binding{};
		vertex_binding.binding = input_resource.location;
		vertex_binding.stride  = attribute.stride;

		vertex_input_state.bindings.push_back(vertex_binding);
	}

	command_buffer.set_vertex_input_state(vertex_input_state);

	// Find submesh vertex buffers matching the shader input attribute names
	for (auto &input_resource : vertex_input_resources)
	{
		const auto &buffer_iter = sub_mesh.vertex_buffers.find(input_resource.name);

		if (buffer_iter != sub_mesh.vertex_buffers.end())
		{
			std::vector<std::reference_wrapper<const core::Buffer>> buffers;
			buffers.emplace_back(std::ref(buffer_iter->second));

			// Bind vertex buffers only for the attribute locations defined
			command_buffer.bind_vertex_buffers(input_resource.location, std::move(buffers), {0});
		}
	}

	// Draw submesh indexed if indices exists
	if (sub_mesh.vertex_indices != 0)
	{
		// Bind index buffer of submesh
		command_buffer.bind_index_buffer(*sub_mesh.index_buffer, sub_mesh.index_offset, sub_mesh.index_type);

		// Draw submesh using indexed data
		command_buffer.draw_indexed(sub_mesh.vertex_indices, 1, 0, 0, 0);
	}
	else
	{
		// Draw submesh using vertices only
		command_buffer.draw(sub_mesh.vertices_count, 1, 0, 0);
	}
}
}        // namespace vkb
