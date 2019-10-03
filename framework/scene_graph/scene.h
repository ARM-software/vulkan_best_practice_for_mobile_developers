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

#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "scene_graph/components/light.h"
#include "scene_graph/components/texture.h"

namespace vkb
{
namespace sg
{
class Node;
class Component;

/// @brief A collection of nodes organized in a tree structure.
///		   It can contain more than one root node.
class Scene
{
  public:
	Scene() = default;

	Scene(const std::string &name);

	void set_name(const std::string &name);

	const std::string &get_name() const;

	void set_nodes(std::vector<std::unique_ptr<Node>> &&nodes);

	void add_node(std::unique_ptr<Node> &&node);

	void add_child(Node &child);

	void add_component(std::unique_ptr<Component> &&component);

	void add_component(std::unique_ptr<Component> &&component, Node &node);

	/**
	 * @brief Set list of components for the given type
	 * @param type_info The type of the component
	 * @param components The list of components (retained)
	 */
	void set_components(const std::type_index &type_info, std::vector<std::unique_ptr<Component>> &&components);

	/**
	 * @brief Set list of components casted from the given template type
	 */
	template <class T>
	void set_components(std::vector<std::unique_ptr<T>> &&components)
	{
		std::vector<std::unique_ptr<Component>> result(components.size());
		std::transform(components.begin(), components.end(), result.begin(),
		               [](std::unique_ptr<T> &component) -> std::unique_ptr<Component> {
			               return std::unique_ptr<Component>(std::move(component));
		               });
		set_components(typeid(T), std::move(result));
	}

	/**
	 * @brief Clears a list of components
	 */
	template <class T>
	void clear_components()
	{
		set_components(typeid(T), {});
	}

	/**
	 * @return List of pointers to components casted to the given template type
	 */
	template <class T>
	std::vector<T *> get_components() const
	{
		std::vector<T *> result;
		if (has_component(typeid(T)))
		{
			auto &scene_components = get_components(typeid(T));

			result.resize(scene_components.size());
			std::transform(scene_components.begin(), scene_components.end(), result.begin(),
			               [](const std::unique_ptr<Component> &component) -> T * {
				               return dynamic_cast<T *>(component.get());
			               });
		}

		return result;
	}

	/**
	 * @return List of components for the given type
	 */
	const std::vector<std::unique_ptr<Component>> &get_components(const std::type_index &type_info) const;

	template <class T>
	bool has_component() const
	{
		return has_component(typeid(T));
	}

	bool has_component(const std::type_index &type_info) const;

	Node *find_node(const std::string &name);

	void set_root_node(Node &node);

	Node &get_root_node();

  private:
	std::string name;

	/// List of all the nodes
	std::vector<std::unique_ptr<Node>> nodes;

	Node *root{nullptr};

	std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> components;
};
}        // namespace sg
}        // namespace vkb
