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

#include "descriptor_set.h"

#include "descriptor_pool.h"
#include "descriptor_set_layout.h"
#include "device.h"

namespace vkb
{
DescriptorSet::DescriptorSet(Device &                                  device,
                             DescriptorSetLayout &                     descriptor_set_layout,
                             const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
                             const BindingMap<VkDescriptorImageInfo> & image_infos) :
    device{device},
    descriptor_set_layout{descriptor_set_layout},
    handle{descriptor_set_layout.get_descriptor_pool().allocate()}
{
	if (!buffer_infos.empty() || !image_infos.empty())
	{
		update(buffer_infos, image_infos);
	}
}

void DescriptorSet::update(const BindingMap<VkDescriptorBufferInfo> &buffer_infos, const BindingMap<VkDescriptorImageInfo> &image_infos)
{
	std::vector<VkWriteDescriptorSet> set_updates;

	// Iterate over all buffer bindings
	for (auto &binding_it : buffer_infos)
	{
		auto  binding         = binding_it.first;
		auto &buffer_bindings = binding_it.second;

		VkDescriptorSetLayoutBinding binding_info;
		if (!descriptor_set_layout.get_layout_binding(binding, binding_info))
		{
			LOGE("Shader layout set does not use buffer binding at #%i", binding);

			continue;
		}

		// Iterate over all binding buffers in array
		for (auto &element_it : buffer_bindings)
		{
			auto  arrayElement = element_it.first;
			auto &buffer_info  = element_it.second;

			VkWriteDescriptorSet write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

			write_descriptor_set.dstBinding      = binding;
			write_descriptor_set.descriptorType  = binding_info.descriptorType;
			write_descriptor_set.pBufferInfo     = &buffer_info;
			write_descriptor_set.dstSet          = handle;
			write_descriptor_set.dstArrayElement = arrayElement;
			write_descriptor_set.descriptorCount = 1;

			set_updates.push_back(write_descriptor_set);
		}
	}

	// Iterate over all image bindings
	for (auto &binding_it : image_infos)
	{
		auto  binding_index     = binding_it.first;
		auto &binding_resources = binding_it.second;

		VkDescriptorSetLayoutBinding binding_info;
		if (!descriptor_set_layout.get_layout_binding(binding_index, binding_info))
		{
			LOGE("Shader layout set does not use image binding at #%i", binding_index);

			continue;
		}

		// Iterate over all binding images in array
		for (auto &element_it : binding_resources)
		{
			auto  arrayElement = element_it.first;
			auto &image_info   = element_it.second;

			VkWriteDescriptorSet write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

			write_descriptor_set.dstBinding      = binding_index;
			write_descriptor_set.descriptorType  = binding_info.descriptorType;
			write_descriptor_set.pImageInfo      = &image_info;
			write_descriptor_set.dstSet          = handle;
			write_descriptor_set.dstArrayElement = arrayElement;
			write_descriptor_set.descriptorCount = 1;

			set_updates.push_back(write_descriptor_set);
		}
	}

	vkUpdateDescriptorSets(device.get_handle(), to_u32(set_updates.size()), set_updates.data(), 0, nullptr);
}

DescriptorSet::DescriptorSet(DescriptorSet &&other) :
    device{other.device},
    descriptor_set_layout{other.descriptor_set_layout},
    handle{other.handle}
{
	other.handle = VK_NULL_HANDLE;
}

DescriptorSet::~DescriptorSet()
{
	// Destroy descriptor set
	if (handle != VK_NULL_HANDLE)
	{
		descriptor_set_layout.get_descriptor_pool().free(handle);
	}
}

VkDescriptorSet DescriptorSet::get_handle() const
{
	return handle;
}
}        // namespace vkb
