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

#include "shader_program.h"

namespace vkb
{
ShaderProgram::ShaderProgram(const std::vector<ShaderModule *> &shader_modules) :
    shader_modules{shader_modules}
{
	// Collect and combine all the shader resources from each of the shader modules
	// Collate them all into a map that is indexed by the name of the resource
	for (auto *shader_module : shader_modules)
	{
		for (const auto &shader_resource : shader_module->get_resources())
		{
			std::string key = shader_resource.name;

			// Since 'Input' and 'Output' resources can have the same name, we modify the key string
			if (shader_resource.type == ShaderResourceType::Input || shader_resource.type == ShaderResourceType::Output)
			{
				key = std::to_string(shader_resource.stages) + "_" + key;
			}

			auto it = resources.find(key);

			if (it != resources.end())
			{
				// Append stage flags if resource already exists
				it->second.stages |= shader_resource.stages;
			}
			else
			{
				// Create a new entry in the map
				resources.emplace(key, shader_resource);
			}
		}
	}

	// Sift through the map of name indexed shader resources
	// Seperate them into their respective sets
	for (auto &it : resources)
	{
		auto &shader_resource = it.second;

		// Find binding by set index in the map.
		auto it2 = sets.find(shader_resource.set);

		if (it2 != sets.end())
		{
			// Add resource to the found set index
			it2->second.push_back(shader_resource);
		}
		else
		{
			// Create a new set index and with the first resource
			sets.emplace(shader_resource.set, std::vector<ShaderResource>{shader_resource});
		}
	}
}

const std::vector<ShaderModule *> &ShaderProgram::get_shader_modules() const
{
	return shader_modules;
}

const std::vector<ShaderResource> ShaderProgram::get_resources(const ShaderResourceType &type, VkShaderStageFlagBits stage) const
{
	std::vector<ShaderResource> found_resources;

	for (auto &it : resources)
	{
		auto &shader_resource = it.second;

		if (shader_resource.type == type || type == ShaderResourceType::All)
		{
			if (shader_resource.stages == stage || stage == VK_SHADER_STAGE_ALL)
			{
				found_resources.push_back(shader_resource);
			}
		}
	}

	return found_resources;
}

const std::unordered_map<uint32_t, std::vector<ShaderResource>> &ShaderProgram::get_shader_sets() const
{
	return sets;
}

}        // namespace vkb
