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

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <glm/glm.hpp>
VKBP_ENABLE_WARNINGS()

#include "platform/filesystem.h"
#include "rendering/pipeline_state.h"
#include "rendering/render_context.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/scene.h"

namespace vkb
{
/**
 * @brief Extracts the extension from an uri
 * @param uri An uniform Resource Identifier
 * @return The extension
 */
std::string get_extension(const std::string &uri);

/**
 * @param name String to convert to snake case
 * @return a snake case version of the string
 */
std::string to_snake_case(const std::string &name);

/**
 * @brief Takes a screenshot of the app by writing the swapchain image to file (slow function)
 * @param filename The name of the file to save the output to
 */
void screenshot(RenderContext &render_context, const std::string &filename);

/**
 * @brief Adds a light to the scene with the specified parameters
 * @param scene The scene to add the light to
 * @param type The light type
 * @param position The position of the light
 * @param rotation The rotation of the light
 * @param props The light properties, such as color and intensity
 * @param parent_node The parent node for the line, defaults to root
 * @return The newly created light component
 */
sg::Light &add_light(sg::Scene &scene, sg::LightType type, const glm::vec3 &position, const glm::quat &rotation = {}, const sg::LightProperties &props = {}, sg::Node *parent_node = nullptr);

/**
 * @brief Adds a point light to the scene with the specified parameters
 * @param scene The scene to add the light to
 * @param position The position of the light
 * @param props The light properties, such as color and intensity
 * @param parent_node The parent node for the line, defaults to root
 * @return The newly created light component
 */
sg::Light &add_point_light(sg::Scene &scene, const glm::vec3 &position, const sg::LightProperties &props = {}, sg::Node *parent_node = nullptr);

/**
 * @brief Adds a directional light to the scene with the specified parameters
 * @param scene The scene to add the light to
 * @param rotation The rotation of the light
 * @param props The light properties, such as color and intensity
 * @param parent_node The parent node for the line, defaults to root
 * @return The newly created light component
 */
sg::Light &add_directional_light(sg::Scene &scene, const glm::quat &rotation, const sg::LightProperties &props = {}, sg::Node *parent_node = nullptr);

/**
 * @brief Add free camera script to a node with a camera object.
 *        Fallback to the default_camera if node not found.
 * @param scene The scene to add the camera to
 * @param node_name The scene node name
 * @return Node where the script was attached as component
 */
sg::Node &add_free_camera(sg::Scene &scene, const std::string &node_name);

}        // namespace vkb
