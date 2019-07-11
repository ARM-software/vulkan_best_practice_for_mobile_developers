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

#include "glsl_compiler.h"

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/OSDependent/osinclude.h>
VKBP_ENABLE_WARNINGS()

namespace vkb
{
namespace
{
inline EShLanguage FindShaderLanguage(VkShaderStageFlagBits stage)
{
	switch (stage)
	{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return EShLangVertex;

		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return EShLangTessControl;

		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return EShLangTessEvaluation;

		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return EShLangGeometry;

		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return EShLangFragment;

		case VK_SHADER_STAGE_COMPUTE_BIT:
			return EShLangCompute;

		default:
			return EShLangVertex;
	}
}
}        // namespace

bool GLSLCompiler::compile_to_spirv(VkShaderStageFlagBits       stage,
                                    const std::vector<uint8_t> &glsl_source,
                                    const std::string &         entry_point,
                                    const ShaderVariant &       shader_variant,
                                    std::vector<std::uint32_t> &spirv,
                                    std::string &               info_log)
{
	// Initialize glslang library.
	glslang::InitializeProcess();

	EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

	EShLanguage language = FindShaderLanguage(stage);
	std::string source   = std::string(glsl_source.begin(), glsl_source.end());

	const char *file_name_list[1] = {""};
	const char *shader_source     = reinterpret_cast<const char *>(source.data());

	glslang::TShader shader(language);
	shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list, 1);
	shader.setEntryPoint(entry_point.c_str());
	shader.setSourceEntryPoint(entry_point.c_str());
	shader.setPreamble(shader_variant.get_preamble().c_str());
	shader.addProcesses(shader_variant.get_processes());

	if (!shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages))
	{
		info_log = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
		return false;
	}

	// Add shader to new program object.
	glslang::TProgram program;
	program.addShader(&shader);

	// Link program.
	if (!program.link(messages))
	{
		info_log = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
		return false;
	}

	// Save any info log that was generated.
	if (shader.getInfoLog())
	{
		info_log += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
	}

	if (program.getInfoLog())
	{
		info_log += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
	}

	glslang::TIntermediate *intermediate = program.getIntermediate(language);

	// Translate to SPIRV.
	if (!intermediate)
	{
		info_log += "Failed to get shared intermediate code.\n";
		return false;
	}

	spv::SpvBuildLogger logger;

	glslang::GlslangToSpv(*intermediate, spirv, &logger);

	info_log += logger.getAllMessages() + "\n";

	// Shutdown glslang library.
	glslang::FinalizeProcess();

	return true;
}
}        // namespace vkb
