#version 320 es
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

layout (set=0, binding=0) uniform sampler2D base_color_sampler;

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

layout (location = 0) out vec4 o_color;

layout(push_constant, std430) uniform PushConstant {
    layout(offset = 128) vec4 light_pos;
    vec4 light_color;
} fs_push_constant;

void main(void)
{
    vec3 normal = normalize(in_normal);

    vec3 world_to_light = fs_push_constant.light_pos.xyz - in_pos.xyz;

    float dist = length(world_to_light) * 0.0001;

    float atten = 1.0 - smoothstep(0.5 * fs_push_constant.light_pos.w, fs_push_constant.light_pos.w, dist);

    world_to_light = normalize(world_to_light);

    float ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);

    vec4 base_color = vec4(1.0, 0.0, 0.0, 1.0);

    base_color = texture(base_color_sampler, in_uv);

    vec4 ambient_color = vec4(0.2, 0.2, 0.2, 1.0) * base_color;

    o_color = ambient_color + ndotl * atten * fs_push_constant.light_color * base_color;
}
