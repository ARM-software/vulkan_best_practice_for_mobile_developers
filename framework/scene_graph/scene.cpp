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

#include <queue>

#include "common/error.h"
#include "component.h"
#include "node.h"

namespace vkb
{
namespace sg
{
Scene::Scene(const std::string &name) :
    name{name}
{}

void Scene::set_name(const std::string &new_name)
{
	name = new_name;
}

const std::string &Scene::get_name() const
{
	return name;
}

void Scene::set_nodes(std::vector<std::unique_ptr<Node>> &&n)
{
	assert(nodes.empty() && "Scene nodes were already set");
	nodes = std::move(n);
}

void Scene::add_node(std::unique_ptr<Node> &&n)
{
	nodes.emplace_back(std::move(n));
}

void Scene::add_child(Node &child)
{
	root->add_child(child);
}

void Scene::add_component(std::unique_ptr<Component> &&component, Node &node)
{
	node.set_component(*component);

	if (component)
	{
		components[component->get_type()].push_back(std::move(component));
	}
}

void Scene::add_component(std::unique_ptr<Component> &&component)
{
	if (component)
	{
		components[component->get_type()].push_back(std::move(component));
	}
}

void Scene::set_components(const std::type_index &type_info, std::vector<std::unique_ptr<Component>> &&new_components)
{
	components[type_info] = std::move(new_components);
}

const std::vector<std::unique_ptr<Component>> &Scene::get_components(const std::type_index &type_info) const
{
	return components.at(type_info);
}

bool Scene::has_component(const std::type_index &type_info) const
{
	auto component = components.find(type_info);
	return (component != components.end() && !component->second.empty());
}

Node *Scene::find_node(const std::string &node_name)
{
	for (auto root_node : root->get_children())
	{
		std::queue<sg::Node *> traverse_nodes{};
		traverse_nodes.push(root_node);

		while (!traverse_nodes.empty())
		{
			auto node = traverse_nodes.front();
			traverse_nodes.pop();

			if (node->get_name() == node_name)
			{
				return node;
			}

			for (auto child_node : node->get_children())
			{
				traverse_nodes.push(child_node);
			}
		}
	}

	return nullptr;
}

void Scene::set_root_node(Node &node)
{
	root = &node;
}

Node &Scene::get_root_node()
{
	return *root;
}
}        // namespace sg
}        // namespace vkb
