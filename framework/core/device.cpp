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

#include "device.h"

#define VMA_IMPLEMENTATION
// Disable warnings for external header
#pragma clang diagnostic ignored "-Wall"
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

namespace vkb
{
Device::Device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const std::vector<const char *> extensions, const VkPhysicalDeviceFeatures &features) :
    physical_device{physical_device}
{
	// Gpu properties
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_family_count, {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO});
	std::vector<std::vector<float>>      queue_priorities(queue_family_count);

	for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_count; ++queue_family_index)
	{
		const VkQueueFamilyProperties &queue_family_property = queue_family_properties[queue_family_index];

		queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 1.0f);

		VkDeviceQueueCreateInfo &queue_create_info = queue_create_infos[queue_family_index];

		queue_create_info.queueFamilyIndex = queue_family_index;
		queue_create_info.queueCount       = queue_family_property.queueCount;
		queue_create_info.pQueuePriorities = queue_priorities[queue_family_index].data();
	}

	VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};

	create_info.pQueueCreateInfos       = queue_create_infos.data();
	create_info.queueCreateInfoCount    = to_u32(queue_create_infos.size());
	create_info.pEnabledFeatures        = &features;
	create_info.enabledExtensionCount   = to_u32(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateDevice(physical_device, &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create device"};
	}

	queues.resize(queue_family_count);

	for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_count; ++queue_family_index)
	{
		const VkQueueFamilyProperties &queue_family_property = queue_family_properties[queue_family_index];

		VkBool32 present_supported{VK_FALSE};
		VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface, &present_supported));

		for (uint32_t queue_index = 0U; queue_index < queue_family_property.queueCount; ++queue_index)
		{
			queues[queue_family_index].emplace_back(*this, queue_family_index, queue_family_property, present_supported, queue_index);
		}
	}

	VmaVulkanFunctions vma_vulkan_func{};
	vma_vulkan_func.vkAllocateMemory                    = vkAllocateMemory;
	vma_vulkan_func.vkBindBufferMemory                  = vkBindBufferMemory;
	vma_vulkan_func.vkBindImageMemory                   = vkBindImageMemory;
	vma_vulkan_func.vkCreateBuffer                      = vkCreateBuffer;
	vma_vulkan_func.vkCreateImage                       = vkCreateImage;
	vma_vulkan_func.vkDestroyBuffer                     = vkDestroyBuffer;
	vma_vulkan_func.vkDestroyImage                      = vkDestroyImage;
	vma_vulkan_func.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
	vma_vulkan_func.vkFreeMemory                        = vkFreeMemory;
	vma_vulkan_func.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
	vma_vulkan_func.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
	vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vma_vulkan_func.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
	vma_vulkan_func.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
	vma_vulkan_func.vkMapMemory                         = vkMapMemory;
	vma_vulkan_func.vkUnmapMemory                       = vkUnmapMemory;

	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.physicalDevice   = physical_device;
	allocator_info.device           = handle;
	allocator_info.pVulkanFunctions = &vma_vulkan_func;

	result = vmaCreateAllocator(&allocator_info, &memory_allocator);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create allocator"};
	}

	command_pool = std::make_unique<CommandPool>(*this, get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0).get_family_index());
	fence_pool   = std::make_unique<FencePool>(*this);
}

Device::~Device()
{
	// Clear caches
	cache_framebuffers.clear();
	cache_render_passes.clear();
	cache_descriptor_sets.clear();
	cache_compute_pipelines.clear();
	cache_graphics_pipelines.clear();
	cache_pipeline_layouts.clear();

	command_pool.reset();
	fence_pool.reset();

	if (memory_allocator != VK_NULL_HANDLE)
	{
		VmaStats stats;
		vmaCalculateStats(memory_allocator, &stats);

		LOGI("Total device memory leaked: {} bytes.", stats.total.usedBytes);

		vmaDestroyAllocator(memory_allocator);
	}

	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyDevice(handle, nullptr);
	}
}

VkPhysicalDevice Device::get_physical_device() const
{
	return physical_device;
}

VkDevice Device::get_handle() const
{
	return handle;
}

VmaAllocator Device::get_memory_allocator() const
{
	return memory_allocator;
}

const VkPhysicalDeviceProperties &Device::get_properties() const
{
	return properties;
}

const Queue &Device::get_queue(uint32_t queue_family_index, uint32_t queue_index)
{
	return queues[queue_family_index][queue_index];
}

const Queue &Device::get_queue_by_flags(VkQueueFlags queue_flags, uint32_t queue_index)
{
	for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
	{
		Queue &first_queue = queues[queue_family_index][0];

		VkQueueFlags queue_flags = first_queue.get_properties().queueFlags;
		uint32_t     queue_count = first_queue.get_properties().queueCount;

		if (((queue_flags & queue_flags) == queue_flags) && queue_index < queue_count)
		{
			return queues[queue_family_index][queue_index];
		}
	}

	throw std::runtime_error("Queue not found");
}

const Queue &Device::get_queue_by_present(uint32_t queue_index)
{
	for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
	{
		Queue &first_queue = queues[queue_family_index][0];

		uint32_t queue_count = first_queue.get_properties().queueCount;

		if (first_queue.support_present() && queue_index < queue_count)
		{
			return queues[queue_family_index][queue_index];
		}
	}

	throw std::runtime_error("Queue not found");
}

CommandBuffer &Device::request_command_buffer()
{
	return command_pool->request_command_buffer();
}

VkFence Device::request_fence()
{
	return fence_pool->request_fence();
}

VkResult Device::wait_idle()
{
	return vkDeviceWaitIdle(handle);
}

PipelineLayout &Device::request_pipeline_layout(std::vector<ShaderModule> &&shader_modules)
{
	return cache_pipeline_layouts.request_resource(*this, std::move(shader_modules));
}

GraphicsPipeline &Device::request_graphics_pipeline(GraphicsPipelineState &                   graphics_state,
                                                    const ShaderStageMap<SpecializationInfo> &specialization_infos)
{
	return cache_graphics_pipelines.request_resource(*this, graphics_state, specialization_infos);
}

ComputePipeline &Device::request_compute_pipeline(const PipelineLayout &    pipeline_layout,
                                                  const SpecializationInfo &specialization_info)
{
	return cache_compute_pipelines.request_resource(*this, pipeline_layout, specialization_info);
}

DescriptorSet &Device::request_descriptor_set(DescriptorSetLayout &                     descriptor_set_layout,
                                              const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
                                              const BindingMap<VkDescriptorImageInfo> & image_infos)
{
	return cache_descriptor_sets.request_resource(*this, descriptor_set_layout, buffer_infos, image_infos);
}

RenderPass &Device::request_render_pass(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
{
	return cache_render_passes.request_resource(*this, attachments, load_store_infos, subpasses);
}

Framebuffer &Device::request_framebuffer(const RenderTarget &render_target, const RenderPass &render_pass)
{
	return cache_framebuffers.request_resource(*this, render_target, render_pass);
}

void Device::clear_framebuffers()
{
	cache_framebuffers.clear();
}

}        // namespace vkb
