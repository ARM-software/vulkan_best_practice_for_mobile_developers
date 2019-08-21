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

#include <iostream>
#include <map>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <variant>

#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>

#include "scene_graph/component.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"
#include "scene_graph/scene.h"

using namespace nlohmann;

namespace vkb
{
namespace utils
{
enum class SceneNodeType
{
	Text,
	Scene,
	Node,
	Transform,
	Mesh,
	SubMesh,
	Texture,
	Material
};

/**
 * @brief SceneNode is a node type used by utils::Graph to create different node variants for different types of scene components.
 * This structure allows for minimum code cluttering when using the graph api.
 * Note: if you want to add a new scene node definition to the graph it must also be defined here
 */
class SceneNode
{
  public:
	enum class Group
	{
		Node,
		Scene,
		Component
	};

	SceneNode()
	{}

	template <typename T>
	SceneNode(T data);

	SceneNode(size_t id, std::string text);

	SceneNode(size_t id, const sg::Scene &scene);
	SceneNode(size_t id, const sg::Node &node);
	SceneNode(size_t id, const sg::Component &component);
	SceneNode(size_t id, const sg::Transform &transform);
	SceneNode(size_t id, const sg::Mesh &mesh);
	SceneNode(size_t id, const sg::SubMesh &submesh);
	SceneNode(size_t id, const sg::Texture &texture, std::string name);
	SceneNode(size_t id, const sg::Material &mat);

	template <typename T>
	static std::string get_id(SceneNodeType type, T value);

	static std::string get_type_str(SceneNodeType type);

	json attributes;

  private:
	static std::unordered_map<SceneNodeType, std::string> SceneNodeTypeStrings;
};
}        // namespace utils
}        // namespace vkb