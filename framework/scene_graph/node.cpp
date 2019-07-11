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
    name{name},
    transform{*this}
{
	set_component(transform);
}

const std::string &Node::get_name() const
{
	return name;
}

void Node::set_parent(Node &p)
{
	parent = &p;

	transform.invalidate_world_matrix();
}

Node *Node::get_parent() const
{
	return parent;
}

void Node::add_child(Node &child)
{
	children.push_back(&child);
}

const std::vector<Node *> &Node::get_children() const
{
	return children;
}

void Node::set_component(Component &component)
{
	auto it = components.find(component.get_type());

	if (it != components.end())
	{
		it->second = &component;
	}
	else
	{
		components.insert(std::make_pair(component.get_type(), &component));
	}
}

Component &Node::get_component(const std::type_index index)
{
	return *components.at(index);
}

bool Node::has_component(const std::type_index index)
{
	return components.count(index) > 0;
}

}        // namespace sg
}        // namespace vkb
