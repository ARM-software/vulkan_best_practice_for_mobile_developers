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
#include "descriptor_pool.h"

namespace vkb
{
class DescriptorSet;
class Device;
class PipelineLayout;

struct ShaderResource;

// Caches DescriptorSet objects for the shader's set index.
// Creates a DescriptorPool to allocate the DescriptorSet objects
class DescriptorSetLayout : public NonCopyable
{
  public:
	DescriptorSetLayout(Device &device, const std::vector<ShaderResource> &set_resources);

	DescriptorSetLayout(DescriptorSetLayout &&other);

	~DescriptorSetLayout();

	VkDescriptorSetLayout get_handle() const;

	DescriptorPool &get_descriptor_pool();

	const std::vector<VkDescriptorSetLayoutBinding> &get_bindings() const;

	bool get_layout_binding(uint32_t binding_index, VkDescriptorSetLayoutBinding &binding) const;

  private:
	Device &device;

	std::unique_ptr<DescriptorPool> descriptor_pool;

	VkDescriptorSetLayout handle{VK_NULL_HANDLE};

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_lookup;
};
}        // namespace vkb
