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

#include "common/helpers.h"
#include "common/vk_common.h"

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

	uint32_t constant_id;

	bool dynamic;

	std::string name;
};

/**
 * @brief Adds support for C style preprocessor macros to glsl shaders
 *        enabling you to define or undefine certain symbols
 */
class ShaderVariant
{
  public:
	ShaderVariant() = default;

	ShaderVariant(std::string &&preamble, std::vector<std::string> &&processes);

	size_t get_id() const;

	/**
	 * @brief Adds a define macro to the shader
	 * @param def String which should go to the right of a define directive
	 */
	void add_define(const std::string &def);

	/**
	 * @brief Adds an undef macro to the shader
	 * @param undef String which should go to the right of an undef directive
	 */
	void add_undefine(const std::string &undef);

	/**
	 * @brief Specifies the size of a named runtime array for automatic reflection. If already specified, overrides the size.
	 * @param runtime_array_name String under which the runtime array is named in the shader
	 * @param size Integer specifying the wanted size of the runtime array (in number of elements, not size in bytes), used for automatic allocation of buffers.
	 * See get_declared_struct_size_runtime_array() in spirv_cross.h
	 */
	void add_runtime_array_size(const std::string &runtime_array_name, size_t size);

	void set_runtime_array_sizes(const std::unordered_map<std::string, size_t> &sizes);

	const std::string &get_preamble() const;

	const std::vector<std::string> &get_processes() const;

	const std::unordered_map<std::string, size_t> &get_runtime_array_sizes() const;

	void clear();

  private:
	size_t id;

	std::string preamble;

	std::vector<std::string> processes;

	std::unordered_map<std::string, size_t> runtime_array_sizes;

	void update_id();
};

class ShaderSource
{
  public:
	ShaderSource() = default;

	ShaderSource(std::vector<uint8_t> &&data);

	size_t get_id() const;

	const std::vector<uint8_t> &get_data() const;

  private:
	size_t id;

	std::vector<uint8_t> data;
};

/**
 * @brief Contains shader code, with an entry point, for a specific shader stage.
 * It is needed by a PipelineLayout to create a Pipeline.
 * ShaderModule can do auto-pairing between shader code and textures.
 * The low level code can change bindings, just keeping the name of the texture.
 * Variants for each texture are also generated, such as HAS_BASE_COLOR_TEX.
 * It works similarly for attribute locations. A current limitation is that only set 0
 * is considered. Uniform buffers are currently hardcoded as well.
 */
class ShaderModule
{
  public:
	ShaderModule(Device &              device,
	             VkShaderStageFlagBits stage,
	             const ShaderSource &  glsl_source,
	             const std::string &   entry_point,
	             const ShaderVariant & shader_variant);

	ShaderModule(const ShaderModule &) = delete;

	ShaderModule(ShaderModule &&other);

	ShaderModule &operator=(const ShaderModule &) = delete;

	ShaderModule &operator=(ShaderModule &&) = delete;

	size_t get_id() const;

	VkShaderStageFlagBits get_stage() const;

	const std::string &get_entry_point() const;

	const std::vector<ShaderResource> &get_resources() const;

	const std::string &get_info_log() const;

	const std::vector<uint32_t> &get_binary() const;

	void set_resource_dynamic(const std::string &resource_name);

  private:
	Device &device;

	/// Shader unique id
	size_t id;

	/// Stage of the shader (vertex, fragment, etc)
	VkShaderStageFlagBits stage{};

	/// Name of the main function
	std::string entry_point;

	/// Compiled source
	std::vector<uint32_t> spirv;

	std::vector<ShaderResource> resources;

	std::string info_log;
};
}        // namespace vkb
