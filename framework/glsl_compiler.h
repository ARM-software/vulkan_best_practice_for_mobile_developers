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
#include <vector>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <glslang/Public/ShaderLang.h>
VKBP_ENABLE_WARNINGS()

#include "common/vk_common.h"
#include "core/shader_module.h"

namespace vkb
{
/// Helper class to generate SPIRV code from GLSL source
/// A very simple version of the glslValidator application
class GLSLCompiler
{
  public:
	/**
	 * @brief Compiles GLSL to SPIRV code
	 * @param stage The Vulkan shader stage flag
	 * @param glsl_source The GLSL source code to be compiled
	 * @param entry_point The entrypoint function name of the shader stage
	 * @param shader_variant The shader variant
	 * @param[out] spirv The generated SPIRV code
	 * @param[out] info_log Stores any log messages during the compilation process
	 */
	bool compile_to_spirv(VkShaderStageFlagBits       stage,
	                      const std::vector<uint8_t> &glsl_source,
	                      const std::string &         entry_point,
	                      const ShaderVariant &       shader_variant,
	                      std::vector<std::uint32_t> &spirv,
	                      std::string &               info_log);
};
}        // namespace vkb
