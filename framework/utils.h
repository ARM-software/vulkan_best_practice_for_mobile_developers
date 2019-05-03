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

#pragma once

#include "common.h"
#include "graphics_pipeline_state.h"
#include "render_context.h"

#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/scene.h"

namespace vkb
{
/**
 * @brief Push constant structure for base.vert shader
 */
struct VertPushConstant
{
	glm::mat4 model;

	glm::mat4 camera_view_proj;
};

/**
 * @brief Push constant structure for base.frag shader
 */
struct FragPushConstant
{
	glm::vec4 light_pos;

	glm::vec4 light_color;
};

/**
 * @brief Extracts the extension from an uri
 * @param uri An uniform Resource Identifier
 * @return The extension
 */
std::string get_extension(const std::string &uri);

/**
 * @brief Helper function to create a shader module from a GLSL source file
 *
 * @param device A Vulkan device and an asset manager already set up
 * @param path The path for the shader (relative to the assets directory)
 *
 * @return The shader module from the given file
 */
ShaderModule create_shader_module(Device &device, const char *path);

/**
 * @brief Helper function to create a pipeline layout with a vertex and fragment shader
 * 
 * @param device A Vulkan device and an asset manager already set up
 * @param vertex_shader_file The path for the vertex shader (relative to the assets directory)
 * @param fragment_shader_file The path for the fragment shader (relative to the assets directory)
 * 
 * @return A pipeline layout object
 */
PipelineLayout &create_pipeline_layout(Device &    device,
                                       const char *vertex_shader_file,
                                       const char *fragment_shader_file);

/**
 * @brief Draw a given submesh
 *
 * @param command_buffer The Vulkan command buffer
 * @param pipeline_layout The Vulkan pipeline layout
 * @param sub_mesh The submesh to render
 */
void draw_scene_submesh(CommandBuffer &command_buffer, PipelineLayout &pipeline_layout, const sg::SubMesh &sub_mesh);

/**
 * @brief Draw each mesh from the scene
 *
 * @param command_buffer The Vulkan command buffer
 * @param pipeline_layout The Vulkan pipeline layout
 * @param scene The scene to render
 */
void draw_scene_meshes(CommandBuffer &command_buffer, PipelineLayout &pipeline_layout, const sg::Scene &scene);

/**
 * @brief Calculates the vulkan style projection matrix
 * 
 * @param proj The projection matrix
 * 
 * @return @ref The vulkan style projection matrix
 */
glm::mat4 vulkan_style_projection(const glm::mat4 &proj);
}        // namespace vkb
