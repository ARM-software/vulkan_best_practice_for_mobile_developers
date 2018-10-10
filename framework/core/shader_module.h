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

#include "common.h"

namespace vkb
{
class Device;

/// Types of shader resources
enum class ShaderResourceType
{
	Input,
	InputAttachment,
	Output,
	Image,
	ImageSampler,
	ImageStorage,
	Sampler,
	BufferUniform,
	BufferStorage,
	PushConstant,
	SpecializationConstant
};

/// Store shader resource data.
/// Used by the shader module.
struct ShaderResource
{
	VkShaderStageFlags stages;

	ShaderResourceType type;

	uint32_t set;

	uint32_t binding;

	uint32_t location;

	uint32_t input_attachment_index;

	uint32_t vec_size;

	uint32_t columns;

	uint32_t array_size;

	uint32_t offset;

	uint32_t size;

	std::string name;
};

class ShaderModule : public NonCopyable
{
  public:
	ShaderModule(Device &                    device,
	             VkShaderStageFlagBits       stage,
	             const std::vector<uint8_t> &glsl_source,
	             const std::string &         entry_point);

	ShaderModule(ShaderModule &&other);

	~ShaderModule();

	VkShaderModule get_handle() const;

	VkShaderStageFlagBits get_stage() const;

	const std::string &get_entry_point() const;

	const std::vector<ShaderResource> &get_resources() const;

	const std::string &get_info_log() const;

	const std::vector<uint32_t> &get_binary() const;

  private:
	Device &device;

	VkShaderModule handle{VK_NULL_HANDLE};

	VkShaderStageFlagBits stage{};

	std::string entry_point;

	std::vector<uint32_t> spirv;

	std::vector<ShaderResource> resources;

	std::string info_log;
};
}        // namespace vkb
