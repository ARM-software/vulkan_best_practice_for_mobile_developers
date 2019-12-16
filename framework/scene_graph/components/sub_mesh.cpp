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

#include "sub_mesh.h"

#include "material.h"
#include "rendering/subpass.h"

namespace vkb
{
namespace sg
{
std::type_index SubMesh::get_type()
{
	return typeid(SubMesh);
}

void SubMesh::set_attribute(const std::string &attribute_name, const VertexAttribute &attribute)
{
	vertex_attributes[attribute_name] = attribute;

	compute_shader_variant();
}

bool SubMesh::get_attribute(const std::string &attribute_name, VertexAttribute &attribute) const
{
	auto attrib_it = vertex_attributes.find(attribute_name);

	if (attrib_it == vertex_attributes.end())
	{
		return false;
	}

	attribute = attrib_it->second;

	return true;
}

void SubMesh::set_material(const Material &new_material)
{
	material = &new_material;

	compute_shader_variant();
}

const Material *SubMesh::get_material() const
{
	return material;
}

const ShaderVariant &SubMesh::get_shader_variant() const
{
	return shader_variant;
}

void SubMesh::compute_shader_variant()
{
	shader_variant.clear();

	if (material != nullptr)
	{
		for (auto &texture : material->textures)
		{
			std::string tex_name = texture.first;
			std::transform(tex_name.begin(), tex_name.end(), tex_name.begin(), ::toupper);

			shader_variant.add_define("HAS_" + tex_name);
		}
	}

	for (auto &attribute : vertex_attributes)
	{
		std::string attrib_name = attribute.first;
		std::transform(attrib_name.begin(), attrib_name.end(), attrib_name.begin(), ::toupper);
		shader_variant.add_define("HAS_" + attrib_name);
	}
}

ShaderVariant &SubMesh::get_mut_shader_variant()
{
	return shader_variant;
}
}        // namespace sg
}        // namespace vkb
