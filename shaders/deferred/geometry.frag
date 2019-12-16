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

#ifdef HAS_BASE_COLOR_TEXTURE
layout (set=0, binding=0) uniform sampler2D base_color_texture;
#endif

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

layout (location = 0) out vec4 o_albedo;
layout (location = 1) out vec4 o_normal;

layout(set = 0, binding = 1) uniform GlobalUniform {
    mat4 model;
    mat4 view_proj;
    vec3 camera_position;
} global_uniform;

layout(push_constant, std430) uniform PBRMaterialUniform {
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
} pbr_material_uniform;

void main(void)
{
    vec3 normal = normalize(in_normal);
    // Transform normals from [-1, 1] to [0, 1]
    o_normal = vec4(0.5 * normal + 0.5, 1.0);

    vec4 base_color = vec4(1.0, 0.0, 0.0, 1.0);

#ifdef HAS_BASE_COLOR_TEXTURE
    base_color = texture(base_color_texture, in_uv);
#else
    base_color = pbr_material_uniform.base_color_factor;
#endif

    o_albedo = base_color;
}
