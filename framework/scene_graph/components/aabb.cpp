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

#include "aabb.h"

namespace vkb
{
namespace sg
{
AABB::AABB()
{
	reset();
}

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max) :
    min{min},
    max{max}
{
}

std::type_index AABB::get_type()
{
	return typeid(AABB);
}

void AABB::update(const glm::vec3 &point)
{
	min = glm::min(min, point);
	max = glm::max(max, point);
}

void AABB::update(const SubMesh &submesh)
{
	// Find vertex position attribute of submesh
	auto position_buffer = submesh.vertex_buffers.find("position");

	if (position_buffer == submesh.vertex_buffers.end())
	{
		LOGW("Submesh {} has no vertex position attributes.", submesh.get_name());

		return;
	}

	// Get buffer data of the vertex position
	const glm::vec3 *vertices = reinterpret_cast<const glm::vec3 *>(position_buffer->second.get_data());

	// Check if submesh is indexed
	if (submesh.vertex_indices > 0)
	{
		const uint16_t *indices = reinterpret_cast<const uint16_t *>(submesh.index_buffer->get_data());

		// Update bounding box for each indexed vertex
		for (uint32_t vertex_id = 0; vertex_id < submesh.vertex_indices; vertex_id++)
		{
			update(vertices[indices[vertex_id]]);
		}
	}
	else
	{
		// Update bounding box for each vertex
		for (uint32_t vertex_id = 0; vertex_id < submesh.vertices_count; vertex_id++)
		{
			update(vertices[vertex_id]);
		}
	}
}

void AABB::transform(glm::mat4 &transform)
{
	min = max = glm::vec4(min, 1.0f) * transform;

	// Update bounding box for the remaining 7 corners of the box
	update(glm::vec4(min.x, min.y, max.z, 1.0f) * transform);
	update(glm::vec4(min.x, max.y, min.z, 1.0f) * transform);
	update(glm::vec4(min.x, max.y, max.z, 1.0f) * transform);
	update(glm::vec4(max.x, min.y, min.z, 1.0f) * transform);
	update(glm::vec4(max.x, min.y, max.z, 1.0f) * transform);
	update(glm::vec4(max.x, max.y, min.z, 1.0f) * transform);
	update(glm::vec4(max, 1.0f) * transform);
}

glm::vec3 AABB::get_scale() const
{
	return (max - min);
}

glm::vec3 AABB::get_center() const
{
	return (min + max) * 0.5f;
}

glm::vec3 AABB::get_min() const
{
	return min;
}

glm::vec3 AABB::get_max() const
{
	return max;
}

void AABB::reset()
{
	min = std::numeric_limits<glm::vec3>::max();

	max = std::numeric_limits<glm::vec3>::min();
}

}        // namespace sg
}        // namespace vkb
