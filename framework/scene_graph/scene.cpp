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

#include "scene.h"

#include "component.h"
#include "node.h"

#include <algorithm>
#include <queue>

namespace vkb
{
namespace sg
{
Scene::Scene(const std::string &name) :
    name{name}
{}

void Scene::set_name(const std::string &name)
{
	this->name = name;
}

const std::string &Scene::get_name() const
{
	return name;
}

void Scene::add_child(std::shared_ptr<Node> child)
{
	if (child)
	{
		children.push_back(child);
	}
}

const std::vector<std::shared_ptr<Node>> &Scene::get_children() const
{
	return children;
}

void Scene::add_component(std::shared_ptr<Component> component)
{
	if (component)
	{
		components[component->get_type()].push_back(component);
	}
}

void Scene::set_components(const std::type_index &type_info, const std::vector<std::shared_ptr<Component>> &components)
{
	this->components[type_info] = components;
}

const std::vector<std::shared_ptr<Component>> &Scene::get_components(const std::type_index &type_info) const
{
	return components.at(type_info);
}

bool Scene::has_component(const std::type_index &type_info) const
{
	return components.count(type_info) != 0 ? true : false;
}

std::shared_ptr<Node> Scene::find_node(const std::string &name)
{
	for (auto root_node : children)
	{
		std::queue<std::shared_ptr<sg::Node>> traverse_nodes{};
		traverse_nodes.push(root_node);

		while (!traverse_nodes.empty())
		{
			auto node = traverse_nodes.front();
			traverse_nodes.pop();

			if (node->get_name() == name)
			{
				return node;
			}

			for (auto child_node : node->get_children())
			{
				traverse_nodes.push(child_node);
			}
		}
	}

	return std::shared_ptr<Node>{};
}
}        // namespace sg
}        // namespace vkb
