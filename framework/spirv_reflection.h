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
#include <vector>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <spirv_glsl.hpp>
VKBP_ENABLE_WARNINGS()

#include "common/vk_common.h"
#include "core/shader_module.h"

namespace vkb
{
/// Generate a list of shader resource based on SPIRV reflection code, and provided ShaderVariant
class SPIRVReflection
{
  public:
	/// @brief Reflects shader resources from SPIRV code
	/// @param stage The Vulkan shader stage flag
	/// @param spirv The SPIRV code of shader
	/// @param[out] resources The list of reflected shader resources
	/// @param variant ShaderVariant used for reflection to specify the size of the runtime arrays in Storage Buffers
	bool reflect_shader_resources(VkShaderStageFlagBits        stage,
	                              const std::vector<uint32_t> &spirv,
	                              std::vector<ShaderResource> &resources,
	                              const ShaderVariant &        variant);

  private:
	void parse_shader_resources(const spirv_cross::Compiler &compiler,
	                            VkShaderStageFlagBits        stage,
	                            std::vector<ShaderResource> &resources,
	                            const ShaderVariant &        variant);

	void parse_push_constants(const spirv_cross::Compiler &compiler,
	                          VkShaderStageFlagBits        stage,
	                          std::vector<ShaderResource> &resources,
	                          const ShaderVariant &        variant);

	void parse_specialization_constants(const spirv_cross::Compiler &compiler,
	                                    VkShaderStageFlagBits        stage,
	                                    std::vector<ShaderResource> &resources,
	                                    const ShaderVariant &        variant);
};
}        // namespace vkb
