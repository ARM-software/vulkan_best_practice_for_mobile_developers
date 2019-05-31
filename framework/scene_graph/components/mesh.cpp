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

#include "mesh.h"

namespace vkb
{
namespace sg
{
Mesh::Mesh(const std::string &name) :
    Component{name}
{}

std::type_index Mesh::get_type()
{
	return typeid(Mesh);
}

const AABB &Mesh::get_bounds() const
{
	return bounds;
}

void Mesh::add_submesh(SubMesh &submesh)
{
	submeshes.push_back(&submesh);

	bounds.update(submesh);
}

const std::vector<SubMesh *> &Mesh::get_submeshes() const
{
	return submeshes;
}

void Mesh::add_node(Node &node)
{
	nodes.push_back(&node);
}

const std::vector<Node *> &Mesh::get_nodes() const
{
	return nodes;
}
}        // namespace sg
}        // namespace vkb
