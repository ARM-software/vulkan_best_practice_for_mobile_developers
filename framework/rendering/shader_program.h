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

#include <unordered_map>
#include <vector>

#include "core/shader_module.h"

namespace vkb
{
/**
 * @brief A ShaderProgram represents the overall group of programmable shaders used in a pipeline
 *        and the underlying organized resources.
 *
 * A shader program ultimately holds the shader sets that are used to create
 * the DescriptorSetLayout's when given to a PipelineLayout.
 */
class ShaderProgram
{
  public:
	ShaderProgram(const std::vector<ShaderModule *> &shader_modules);

	ShaderProgram(const ShaderProgram &) = delete;

	ShaderProgram(ShaderProgram &&other) = default;

	~ShaderProgram() = default;

	ShaderProgram &operator=(const ShaderProgram &) = delete;

	ShaderProgram &operator=(ShaderProgram &&) = delete;

	const std::vector<ShaderModule *> &get_shader_modules() const;

	const std::vector<ShaderResource> get_resources(const ShaderResourceType &type = ShaderResourceType::All, VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;

	const std::unordered_map<uint32_t, std::vector<ShaderResource>> &get_shader_sets() const;

  private:
	// The shader modules that this program uses
	std::vector<ShaderModule *> shader_modules;

	// The shader resources that this program uses, indexed by their name
	std::unordered_map<std::string, ShaderResource> resources;

	// A map of each set and the resources it owns used by the shader program
	std::unordered_map<uint32_t, std::vector<ShaderResource>> sets;
};
}        // namespace vkb
