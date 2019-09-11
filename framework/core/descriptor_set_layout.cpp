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

#include "descriptor_set_layout.h"

#include "device.h"
#include "shader_module.h"

namespace vkb
{
namespace
{
inline VkDescriptorType find_descriptor_type(ShaderResourceType resource_type, bool dynamic)
{
	switch (resource_type)
	{
		case ShaderResourceType::InputAttachment:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			break;
		case ShaderResourceType::Image:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			break;
		case ShaderResourceType::ImageSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		case ShaderResourceType::ImageStorage:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			break;
		case ShaderResourceType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
			break;
		case ShaderResourceType::BufferUniform:
			if (dynamic)
			{
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			}
			else
			{
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
			break;
		case ShaderResourceType::BufferStorage:
			if (dynamic)
			{
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			}
			else
			{
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			}
			break;
		default:
			throw std::runtime_error("No conversion possible for the shader resource type.");
			break;
	}
}
}        // namespace

DescriptorSetLayout::DescriptorSetLayout(Device &device, const std::vector<ShaderResource> &set_resources) :
    device{device}
{
	for (auto &resource : set_resources)
	{
		// Skip shader resources whitout a binding point
		if (resource.type == ShaderResourceType::Input ||
		    resource.type == ShaderResourceType::Output ||
		    resource.type == ShaderResourceType::PushConstant ||
		    resource.type == ShaderResourceType::SpecializationConstant)
		{
			continue;
		}

		// Convert from ShaderResourceType to VkDescriptorType.
		auto descriptor_type = find_descriptor_type(resource.type, resource.dynamic);

		// Convert ShaderResource to VkDescriptorSetLayoutBinding
		VkDescriptorSetLayoutBinding layout_binding{};

		layout_binding.binding         = resource.binding;
		layout_binding.descriptorCount = resource.array_size;
		layout_binding.descriptorType  = descriptor_type;
		layout_binding.stageFlags      = static_cast<VkShaderStageFlags>(resource.stages);

		bindings.push_back(layout_binding);

		// Store mapping between binding and the binding point
		bindings_lookup.emplace(resource.binding, layout_binding);

		resources_lookup.emplace(resource.name, resource.binding);
	}

	VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

	create_info.bindingCount = to_u32(bindings.size());
	create_info.pBindings    = bindings.data();

	// Create the Vulkan descriptor set layout handle
	VkResult result = vkCreateDescriptorSetLayout(device.get_handle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create DescriptorSetLayout"};
	}
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout &&other) :
    device{other.device},
    handle{other.handle},
    bindings{std::move(other.bindings)},
    bindings_lookup{std::move(other.bindings_lookup)},
    resources_lookup{std::move(other.resources_lookup)}
{
	other.handle = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	// Destroy descriptor set layout
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device.get_handle(), handle, nullptr);
	}
}

VkDescriptorSetLayout DescriptorSetLayout::get_handle() const
{
	return handle;
}

const std::vector<VkDescriptorSetLayoutBinding> &DescriptorSetLayout::get_bindings() const
{
	return bindings;
}

bool DescriptorSetLayout::get_layout_binding(uint32_t binding_index, VkDescriptorSetLayoutBinding &binding) const
{
	auto it = bindings_lookup.find(binding_index);

	if (it == bindings_lookup.end())
	{
		return false;
	}

	binding = it->second;

	return true;
}

bool DescriptorSetLayout::has_layout_binding(const std::string &name, VkDescriptorSetLayoutBinding &binding) const
{
	auto it = resources_lookup.find(name);

	if (it == resources_lookup.end())
	{
		return false;
	}

	return get_layout_binding(it->second, binding);
}
}        // namespace vkb
