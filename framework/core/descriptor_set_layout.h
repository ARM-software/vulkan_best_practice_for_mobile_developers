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
class DescriptorPool;
class Device;

struct ShaderResource;

/**
 * @brief Caches DescriptorSet objects for the shader's set index.
 *        Creates a DescriptorPool to allocate the DescriptorSet objects
 */
class DescriptorSetLayout
{
  public:
	/**
	 * @brief Creates a descriptor set layout from a set of resources
	 * @param device A valid Vulkan device
	 * @param resource_set A grouping of shader resources belonging to the same set
	 * @param use_dynamic_resources Whether to set the resources to dynamic (where applicable)
	 */
	DescriptorSetLayout(Device &device, const std::vector<ShaderResource> &resource_set, bool use_dynamic_resources);

	DescriptorSetLayout(const DescriptorSetLayout &) = delete;

	DescriptorSetLayout(DescriptorSetLayout &&other);

	~DescriptorSetLayout();

	DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

	DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

	VkDescriptorSetLayout get_handle() const;

	const std::vector<VkDescriptorSetLayoutBinding> &get_bindings() const;

	std::unique_ptr<VkDescriptorSetLayoutBinding> get_layout_binding(uint32_t binding_index) const;

	std::unique_ptr<VkDescriptorSetLayoutBinding> get_layout_binding(const std::string &name) const;

  private:
	Device &device;

	VkDescriptorSetLayout handle{VK_NULL_HANDLE};

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_lookup;

	std::unordered_map<std::string, uint32_t> resources_lookup;
};
}        // namespace vkb
