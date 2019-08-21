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

#include <string>
#include <unordered_map>

#include <volk.h>

namespace vkb
{
enum class ShaderResourceType;

namespace sg
{
enum class AlphaMode;
}

namespace utils
{
extern std::string vk_result_to_string(VkResult result);

extern std::string to_string(VkFormat format);

extern std::string to_string(VkSampleCountFlagBits flags);

extern std::string to_string_shader_stage_flags(VkShaderStageFlags flags);

extern std::string to_string(VkPhysicalDeviceType type);

extern std::string to_string(VkSurfaceTransformFlagBitsKHR flags);

extern std::string to_string(VkPresentModeKHR mode);

extern std::string to_string_vk_image_usage_flags(VkImageUsageFlags flags);

extern std::string to_string_vk_image_aspect_flags(VkImageAspectFlags flags);

extern std::string to_string(VkImageTiling tiling);

extern std::string to_string(VkImageType type);

extern std::string to_string(VkExtent2D format);

extern std::string to_string(VkBlendFactor blend);

extern std::string to_string(VkVertexInputRate rate);

extern std::string to_string_vk_bool(VkBool32 state);

extern std::string to_string(VkPrimitiveTopology topology);

extern std::string to_string(VkFrontFace face);

extern std::string to_string(VkPolygonMode mode);

extern std::string to_string_vk_cull_mode_flags(VkCullModeFlags flags);

extern std::string to_string(VkCompareOp operation);

extern std::string to_string(VkStencilOp operation);

extern std::string to_string(VkLogicOp operation);

extern std::string to_string(VkBlendOp operation);

extern std::string to_string_vk_color_component_flags(VkColorComponentFlags operation);

extern std::string to_string(sg::AlphaMode mode);

extern std::string to_string(bool flag);

extern std::string to_string(ShaderResourceType type);

extern std::unordered_map<VkFormat, std::string> vk_format_strings;

}        // namespace utils
}        // namespace vkb