/* Copyright (c) 2018-2019, Arm Limited and Contributors
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

#include "utils.h"

#include "core/pipeline_layout.h"
#include "core/shader_module.h"

#include "scene_graph/components/image.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/sampler.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"

#include <queue>

namespace vkb
{
namespace
{
VkShaderStageFlagBits find_shader_stage(const std::string &ext)
{
	if (ext == "vert")
	{
		return VK_SHADER_STAGE_VERTEX_BIT;
	}
	else if (ext == "frag")
	{
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	else if (ext == "comp")
	{
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
	else if (ext == "geom")
	{
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	}
	else if (ext == "tesc")
	{
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	}
	else if (ext == "tese")
	{
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}

	throw std::runtime_error("File extension `" + ext + "` does not have a vulkan shader stage.");
};
}        // namespace

ShaderModule create_shader_module(Device &device, const char *path)
{
	std::string file_ext = path;

	file_ext = file_ext.substr(file_ext.find_last_of(".") + 1);

	auto shader_stage = find_shader_stage(file_ext);

	auto buffer = read_binary_file(path);

	return ShaderModule{device, shader_stage, buffer, "main"};
}

PipelineLayout &create_pipeline_layout(Device &    device,
                                       const char *vertex_shader_file,
                                       const char *fragment_shader_file)
{
	std::vector<ShaderModule> shader_modules;
	shader_modules.push_back(create_shader_module(device, vertex_shader_file));
	shader_modules.push_back(create_shader_module(device, fragment_shader_file));

	return device.request_pipeline_layout(std::move(shader_modules));
}

void draw_scene_submesh(CommandBuffer &command_buffer, PipelineLayout &pipeline_layout, const sg::SubMesh &sub_mesh)
{
	auto &material = sub_mesh.material;

	auto &base_color_texture = material->base_color_texture;

	// Bind color texture of material
	if (base_color_texture && base_color_texture->get_image() && base_color_texture->get_sampler())
	{
		command_buffer.bind_image(*base_color_texture->get_image()->image_view,
		                          base_color_texture->get_sampler()->vk_sampler, 0, 0, 0);
	}

	auto vertex_input_resources = pipeline_layout.get_vertex_input_attributes();

	VertexInputState vertex_input_state;

	for (auto &input_resource : vertex_input_resources)
	{
		auto attribute_it = sub_mesh.vertex_attributes.find(input_resource.name);

		if (attribute_it == sub_mesh.vertex_attributes.end())
		{
			continue;
		}

		VkVertexInputAttributeDescription vertex_attribute{};
		vertex_attribute.binding  = input_resource.location;
		vertex_attribute.format   = attribute_it->second.format;
		vertex_attribute.location = input_resource.location;
		vertex_attribute.offset   = attribute_it->second.offset;

		vertex_input_state.attributes.push_back(vertex_attribute);

		VkVertexInputBindingDescription vertex_binding{};
		vertex_binding.binding = input_resource.location;
		vertex_binding.stride  = attribute_it->second.stride;

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

void draw_scene_meshes(CommandBuffer &command_buffer, PipelineLayout &pipeline_layout, const sg::Scene &scene)
{
	auto meshes = scene.get_components<sg::Mesh>();

	// draw all meshes in the scene
	for (auto &mesh : meshes)
	{
		// draw mesh for each node
		for (auto &node : mesh->get_nodes())
		{
			auto &transform = node->get_component<vkb::sg::Transform>();

			// set world matrix of the node
			command_buffer.push_constants(0, transform.get_world_matrix());

			// draw each submesh of the current mesh
			for (auto &sub_mesh : mesh->get_submeshes())
			{
				draw_scene_submesh(command_buffer, pipeline_layout, *sub_mesh);
			}
		}
	}
}

glm::mat4 vulkan_style_projection(const glm::mat4 &proj)
{
	// Flip Y in clipspace. X = -1, Y = -1 is topLeft in Vulkan.
	glm::mat4 mat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

	// Z depth is [0, 1] range instead of [-1, 1].
	mat = glm::scale(mat, glm::vec3(1.0f, 1.0f, 0.5f));

	return glm::translate(mat, glm::vec3(0.0f, 0.0f, 1.0f)) * proj;
}
}        // namespace vkb
