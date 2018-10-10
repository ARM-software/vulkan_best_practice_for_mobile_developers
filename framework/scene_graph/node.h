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

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace vkb
{
namespace sg
{
class Component;
class Transform;

/// @brief A leaf of the tree structure which can have children and a single parent.
class Node
{
  public:
	Node(const std::string &name);

	virtual ~Node() = default;

	const std::string &get_name() const;

	void set_parent(std::shared_ptr<Node> parent);

	std::shared_ptr<Node> get_parent();

	void add_child(std::shared_ptr<Node> child);

	const std::vector<std::shared_ptr<Node>> &get_children();

	void set_component(std::shared_ptr<Component> component);

	template <class T>
	inline std::shared_ptr<T> get_component()
	{
		return std::dynamic_pointer_cast<T>(get_component(typeid(T)));
	}

	std::shared_ptr<Component> get_component(std::type_index typeIndex);

	template <class T>
	bool has_component()
	{
		return has_component(typeid(T));
	}

	bool has_component(std::type_index index);

  private:
	std::string name;

	std::shared_ptr<Node> parent;

	std::vector<std::shared_ptr<Node>> children;

	std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
};
}        // namespace sg
}        // namespace vkb
