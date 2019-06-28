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

#include "shader_module.h"

#include "common/logging.h"
#include "device.h"
#include "glsl_compiler.h"
#include "spirv_reflection.h"

namespace vkb
{
ShaderModule::ShaderModule(Device &device, VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const std::string &entry_point, const ShaderVariant &shader_variant) :
    device{device},
    stage{stage},
    entry_point{entry_point}
{
	// Check if application is passing in GLSL source code to compile to SPIR-V
	if (glsl_source.get_data().empty())
	{
		throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
	}

	// Compiling from GLSL source requires the entry point
	if (entry_point.empty())
	{
		throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
	}

	GLSLCompiler glsl_compiler;

	// Compile the GLSL source
	if (!glsl_compiler.compile_to_spirv(stage, glsl_source.get_data(), entry_point, shader_variant, spirv, info_log))
	{
		throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
	}

	SPIRVReflection spirv_reflection;

	// Reflect all shader resouces
	if (!spirv_reflection.reflect_shader_resources(stage, spirv, resources))
	{
		throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
	}

	// Generate a unique id, determined by source and variant
	std::hash<std::string> hasher{};
	id = hasher(std::string{spirv.cbegin(), spirv.cend()});
}

ShaderModule::ShaderModule(ShaderModule &&other) :
    device{other.device},
    id{other.id},
    stage{other.stage},
    entry_point{other.entry_point},
    spirv{other.spirv},
    resources{other.resources},
    info_log{other.info_log}
{
	other.stage = {};
}

size_t ShaderModule::get_id() const
{
	return id;
}

VkShaderStageFlagBits ShaderModule::get_stage() const
{
	return stage;
}

const std::string &ShaderModule::get_entry_point() const
{
	return entry_point;
}

const std::vector<ShaderResource> &ShaderModule::get_resources() const
{
	return resources;
}

const std::string &ShaderModule::get_info_log() const
{
	return info_log;
}

const std::vector<uint32_t> &ShaderModule::get_binary() const
{
	return spirv;
}

void ShaderModule::set_resource_dynamic(const std::string &resource_name)
{
	auto it = std::find_if(resources.begin(), resources.end(), [&resource_name](const ShaderResource &resource) { return resource.name == resource_name; });

	if (it != resources.end())
	{
		if (it->type == ShaderResourceType::BufferUniform || it->type == ShaderResourceType::BufferStorage)
		{
			it->dynamic = true;
		}
		else
		{
			LOGW("Resource `{}` does not support dynamic.", resource_name);
		}
	}
	else
	{
		LOGW("Resource `{}` not found for shader.", resource_name);
	}
}

ShaderVariant::ShaderVariant(std::string &&preamble, std::vector<std::string> &&processes) :
    preamble{std::move(preamble)},
    processes{std::move(processes)}
{
	update_id();
}

size_t ShaderVariant::get_id() const
{
	return id;
}

void ShaderVariant::add_define(const std::string &def)
{
	processes.push_back("D" + def);

	std::string tmp_def = def;

	// The "=" needs to turn into a space
	size_t pos_equal = tmp_def.find_first_of("=");
	if (pos_equal != std::string::npos)
	{
		tmp_def[pos_equal] = ' ';
	}

	preamble.append("#define " + tmp_def + "\n");

	update_id();
}

void ShaderVariant::add_undefine(const std::string &undef)
{
	processes.push_back("U" + undef);

	preamble.append("#undef " + undef + "\n");

	update_id();
}

const std::string &ShaderVariant::get_preamble() const
{
	return preamble;
}

const std::vector<std::string> &ShaderVariant::get_processes() const
{
	return processes;
}

void ShaderVariant::clear()
{
	preamble.clear();
	processes.clear();
	update_id();
}

void ShaderVariant::update_id()
{
	std::hash<std::string> hasher{};
	id = hasher(preamble);
}

ShaderSource::ShaderSource(std::vector<uint8_t> &&data) :
    data{std::move(data)}
{
	std::hash<std::string> hasher{};
	id = hasher(std::string{this->data.cbegin(), this->data.cend()});
}

size_t ShaderSource::get_id() const
{
	return id;
}

const std::vector<uint8_t> &ShaderSource::get_data() const
{
	return data;
}
}        // namespace vkb
