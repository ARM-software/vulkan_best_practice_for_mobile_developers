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

	void add_child(std::shared_ptr<Node> child);

	const std::vector<std::shared_ptr<Node>> &get_children() const;

	void add_component(std::shared_ptr<Component> component);

	/**
	 * @brief Set list of components for the given type
	 */
	void set_components(const std::type_index &type_info, const std::vector<std::shared_ptr<Component>> &components);

	/**
	 * @brief Set list of components casted from the given template type
	 */
	template <class T>
	void set_components(const std::vector<std::shared_ptr<T>> &components)
	{
		std::vector<std::shared_ptr<Component>> result(components.size());
		std::transform(components.begin(), components.end(), result.begin(),
		               [](std::shared_ptr<T> component) -> std::shared_ptr<Component> {
			               return std::dynamic_pointer_cast<Component>(component);
		               });

		set_components(typeid(T), result);
	}

	/**
	 * @return List of components casted to the given template type
	 */
	template <class T>
	std::vector<std::shared_ptr<T>> get_components() const
	{
		auto components = get_components(typeid(T));

		std::vector<std::shared_ptr<T>> result(components.size());

		std::transform(components.begin(), components.end(), result.begin(),
		               [](std::shared_ptr<Component> component) -> std::shared_ptr<T> {
			               return std::dynamic_pointer_cast<T>(component);
		               });

		return result;
	}

	/**
	 * @return List of components for the given type
	 */
	const std::vector<std::shared_ptr<Component>> &get_components(const std::type_index &type_info) const;

	template <class T>
	bool has_component() const
	{
		return has_component(typeid(T));
	}

	bool has_component(const std::type_index &type_info) const;

	std::shared_ptr<Node> find_node(const std::string &name);

  private:
	std::string name;

	std::vector<std::shared_ptr<Node>> children;

	std::unordered_map<std::type_index, std::vector<std::shared_ptr<Component>>> components;
};
}        // namespace sg
}        // namespace vkb
