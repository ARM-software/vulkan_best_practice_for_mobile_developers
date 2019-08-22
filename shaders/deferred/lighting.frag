#version 450
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

precision highp float;

layout (input_attachment_index = 0, binding = 0) uniform subpassInput i_depth;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput i_albedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput i_normal;

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 o_color;


layout(set = 0, binding = 3) uniform GlobalUniform {
	mat4 inv_view_proj;
	vec4 light_pos;
	vec4 light_color;
	vec2 inv_resolution;
} global_uniform;

void main()
{
	// Retrieve position from depth
	vec4 clip = vec4(gl_FragCoord.xy * global_uniform.inv_resolution * 2.0 - 1.0, subpassLoad(i_depth).x, 1.0);
	highp vec4 world_w = global_uniform.inv_view_proj * clip;
	highp vec3 pos = world_w.xyz / world_w.w;

	vec4 albedo = subpassLoad(i_albedo);
	// Transform from [0,1] to [-1,1]
	vec3 normal = subpassLoad(i_normal).xyz;
	normal = 2.0 * normal - 1.0;

	// Calculate lighting
	vec4 L = vec4(0.0);

	for (int i = -4 ; i < 4; ++i)
	{
		for(int j = 0; j < 2; ++j)
		{
			vec3 light_pos = global_uniform.light_pos.xyz;
			light_pos.x += i * 400;
			light_pos.z += j * (225 + 140);
			light_pos.y = 8;
			vec3 world_to_light = light_pos - pos;

			float dist = length(world_to_light) * 0.005;

			float atten = 1.0 / (1.0 + dist * dist);

			world_to_light = normalize(world_to_light);

			float ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);

			L += ndotl * atten * global_uniform.light_color;
		}
	}

	vec4 ambient_color = vec4(0.2, 0.2, 0.2, 1.0) * albedo;

	o_color = L * albedo;
	o_color.a = 1.0;
}
