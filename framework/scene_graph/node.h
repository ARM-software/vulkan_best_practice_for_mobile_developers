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

#include "scene_graph/components/transform.h"

namespace vkb
{
namespace sg
{
class Component;

/// @brief A leaf of the tree structure which can have children and a single parent.
class Node
{
  public:
	Node(const std::string &name);

	virtual ~Node() = default;

	const std::string &get_name() const;

	Transform &get_transform()
	{
		return transform;
	}

	void set_parent(Node &parent);

	Node *get_parent() const;

	void add_child(Node &child);

	const std::vector<Node *> &get_children() const;

	void set_component(Component &component);

	template <class T>
	inline T &get_component()
	{
		return dynamic_cast<T &>(get_component(typeid(T)));
	}

	Component &get_component(const std::type_index index);

	template <class T>
	bool has_component()
	{
		return has_component(typeid(T));
	}

	bool has_component(const std::type_index index);

  private:
	std::string name;

	sg::Transform transform;

	Node *parent{nullptr};

	std::vector<Node *> children;

	std::unordered_map<std::type_index, Component *> components;
};
}        // namespace sg
}        // namespace vkb
