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
#include <typeinfo>
#include <vector>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <glm/glm.hpp>
VKBP_ENABLE_WARNINGS()

#include "scene_graph/component.h"
#include "scene_graph/components/sub_mesh.h"

namespace vkb
{
namespace sg
{
/**
 * @brief Axis Aligned Bounding Box
 */
class AABB : public Component
{
  public:
	AABB();

	AABB(const glm::vec3 &min, const glm::vec3 &max);

	virtual ~AABB() = default;

	virtual std::type_index get_type() override;

	/**
	 * @brief Update the bounding box based on the given vertex position
	 * @param point The 3D position of a point
	 */
	void update(const glm::vec3 &point);

	/**
	 * @brief Update the bounding box based on the given submesh vertices
	 * @param submesh The submesh object
	 */
	void update(SubMesh &submesh);

	/**
	 * @brief Apply a given matrix transformation to the bounding box
	 * @param transform The matrix transform to apply
	 */
	void transform(glm::mat4 &transform);

	/**
	 * @brief Scale vector of the bounding box
	 * @return vector in 3D space
	 */
	glm::vec3 get_scale() const;

	/**
	 * @brief Center position of the bounding box
	 * @return vector in 3D space
	 */
	glm::vec3 get_center() const;

	/**
	 * @brief Minimum position of the bounding box
	 * @return vector in 3D space
	 */
	glm::vec3 get_min() const;

	/**
	 * @brief Maximum position of the bounding box
	 * @return vector in 3D space
	 */
	glm::vec3 get_max() const;

	/**
	 * @brief Resets the min and max position coordinates
	 */
	void reset();

  private:
	glm::vec3 min;

	glm::vec3 max;
};
}        // namespace sg
}        // namespace vkb
