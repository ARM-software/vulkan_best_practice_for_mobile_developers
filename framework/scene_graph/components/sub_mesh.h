/* Copyright (c) 2018-2019, Arm Limited and Contributors
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

#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "common/vk_common.h"
#include "core/buffer.h"
#include "core/shader_module.h"
#include "scene_graph/component.h"

namespace vkb
{
namespace sg
{
class Material;

struct VertexAttribute
{
	VkFormat format = VK_FORMAT_UNDEFINED;

	std::uint32_t stride = 0;

	std::uint32_t offset = 0;
};

class SubMesh : public Component
{
  public:
	virtual ~SubMesh() = default;

	virtual std::type_index get_type() override;

	VkIndexType index_type{};

	std::uint32_t index_offset = 0;

	std::uint32_t vertices_count = 0;

	std::uint32_t vertex_indices = 0;

	std::unordered_map<std::string, core::Buffer> vertex_buffers;

	std::unique_ptr<core::Buffer> index_buffer;

	void set_attribute(const std::string &name, const VertexAttribute &attribute);

	bool get_attribute(const std::string &name, VertexAttribute &attribute) const;

	void set_material(const Material &material);

	const Material *get_material() const;

	const ShaderVariant &get_shader_variant() const;

  private:
	std::unordered_map<std::string, VertexAttribute> vertex_attributes;

	const Material *material{nullptr};

	ShaderVariant shader_variant;

	void compute_shader_variant();
};
}        // namespace sg
}        // namespace vkb
