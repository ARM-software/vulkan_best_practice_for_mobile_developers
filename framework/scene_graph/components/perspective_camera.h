/* Copyright (c) 2019, Arm Limited and Contributors
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
#include <unordered_map>
#include <vector>

#include "common.h"
#include "scene_graph/components/camera.h"

namespace vkb
{
namespace sg
{
class PerspectiveCamera : public Camera
{
  public:
	PerspectiveCamera(const std::string &name);

	virtual ~PerspectiveCamera() = default;

	void set_aspect_ratio(float aspect_ratio);

	void set_field_of_view(float fov);

	void set_far_plane(float zfar);

	void set_near_plane(float znear);

	float get_aspect_ratio();

	float get_field_of_view();

	virtual glm::mat4 get_projection() override;

  private:
	/**
	 * @brief Screen size aspect ratio
	 */
	float aspect_ratio{1.0f};

	/**
	 * @brief Horizontal field of view in radians
	 */
	float fov{glm::radians(60.0f)};

	float far_plane{100.0};

	float near_plane{0.1f};
};
}        // namespace sg
}        // namespace vkb
