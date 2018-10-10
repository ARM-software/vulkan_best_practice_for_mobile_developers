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

#include "node.h"

#include "component.h"

#include "components/transform.h"

namespace vkb
{
namespace sg
{
Node::Node(const std::string &name) :
    name{name}
{
}

const std::string &Node::get_name() const
{
	return name;
}

void Node::set_parent(std::shared_ptr<Node> parent)
{
	if (parent)
	{
		this->parent = parent;

		if (has_component<Transform>())
		{
			get_component<Transform>()->invalidate_world_matrix();
		}
	}
}

std::shared_ptr<Node> Node::get_parent()
{
	return parent;
}

void Node::add_child(std::shared_ptr<Node> child_node)
{
	if (child_node)
	{
		children.push_back(child_node);
	}
}

const std::vector<std::shared_ptr<Node>> &Node::get_children()
{
	return children;
}

void Node::set_component(std::shared_ptr<Component> component)
{
	if (component)
	{
		auto attachment_iter = components.find(component->get_type());

		if (attachment_iter != components.end())
		{
			attachment_iter->second = component;
		}
		else
		{
			components.insert(std::make_pair(component->get_type(), component));
		}
	}
}

std::shared_ptr<Component> Node::get_component(std::type_index type_index)
{
	return components.at(type_index);
}

bool Node::has_component(std::type_index type_index)
{
	return (components.count(type_index) ? true : false);
}
}        // namespace sg
}        // namespace vkb
