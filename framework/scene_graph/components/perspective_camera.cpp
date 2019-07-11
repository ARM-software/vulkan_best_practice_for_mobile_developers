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

#include "perspective_camera.h"

namespace vkb
{
namespace sg
{
PerspectiveCamera::PerspectiveCamera(const std::string &name) :
    Camera{name}
{}

void PerspectiveCamera::set_field_of_view(float new_fov)
{
	fov = new_fov;
}

void PerspectiveCamera::set_far_plane(float zfar)
{
	far_plane = zfar;
}

void PerspectiveCamera::set_near_plane(float znear)
{
	near_plane = znear;
}

void PerspectiveCamera::set_aspect_ratio(float new_aspect_ratio)
{
	aspect_ratio = new_aspect_ratio;
}

float PerspectiveCamera::get_field_of_view()
{
	/* Calculate vertical fov */
	auto vfov = static_cast<float>(2 * atan(tan(fov / 2) * (1.0 / aspect_ratio)));

	return aspect_ratio > 1.0f ? fov : vfov;
}

float PerspectiveCamera::get_aspect_ratio()
{
	return aspect_ratio;
}

glm::mat4 PerspectiveCamera::get_projection()
{
	return glm::perspective(get_field_of_view(), aspect_ratio, near_plane, far_plane);
}
}        // namespace sg
}        // namespace vkb
