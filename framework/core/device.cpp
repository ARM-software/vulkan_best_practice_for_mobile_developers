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
#include <vk_mem_alloc.h>

namespace vkb
{
Device::Device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<const char *> extensions, const VkPhysicalDeviceFeatures &features) :
    physical_device{physical_device},
    resource_cache{*this}
{
	// Gpu properties
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	uint32_t queue_family_properties_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_family_properties.data());

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_family_properties_count, {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO});
	std::vector<std::vector<float>>      queue_priorities(queue_family_properties_count);

	for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties_count; ++queue_family_index)
	{
		const VkQueueFamilyProperties &queue_family_property = queue_family_properties[queue_family_index];

		queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 1.0f);

		VkDeviceQueueCreateInfo &queue_create_info = queue_create_infos[queue_family_index];

		queue_create_info.queueFamilyIndex = queue_family_index;
		queue_create_info.queueCount       = queue_family_property.queueCount;
		queue_create_info.pQueuePriorities = queue_priorities[queue_family_index].data();
	}

	// Check extensions to enable Vma Dedicated Allocation
	uint32_t device_extension_count;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr));
	std::vector<VkExtensionProperties> device_extensions(device_extension_count);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.data()));

	bool can_get_memory_requirements = std::find_if(std::begin(device_extensions),
	                                                std::end(device_extensions),
	                                                [](auto &extension) { return std::strcmp(extension.extensionName, "VK_KHR_get_memory_requirements2") == 0; }) != std::end(device_extensions);
	bool has_dedicated_allocation    = std::find_if(std::begin(device_extensions),
                                                 std::end(device_extensions),
                                                 [](auto &extension) { return std::strcmp(extension.extensionName, "VK_KHR_dedicated_allocation") == 0; }) != std::end(device_extensions);

	if (can_get_memory_requirements && has_dedicated_allocation)
	{
		extensions.push_back("VK_KHR_get_memory_requirements2");
		extensions.push_back("VK_KHR_dedicated_allocation");
		LOGI("Dedicated Allocation enabled");
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

	queues.resize(queue_family_properties_count);

	for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties_count; ++queue_family_index)
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
	allocator_info.physicalDevice = physical_device;
	allocator_info.device         = handle;

	if (can_get_memory_requirements && has_dedicated_allocation)
	{
		allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
		vma_vulkan_func.vkGetImageMemoryRequirements2KHR  = vkGetImageMemoryRequirements2KHR;
	}

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
	resource_cache.clear();

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

const VkPhysicalDeviceFeatures &Device::get_features() const
{
	return features;
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

bool Device::is_image_format_supported(VkFormat format) const
{
	VkImageFormatProperties format_properties;

	auto result = vkGetPhysicalDeviceImageFormatProperties(physical_device,
	                                                       format,
	                                                       VK_IMAGE_TYPE_2D,
	                                                       VK_IMAGE_TILING_OPTIMAL,
	                                                       VK_IMAGE_USAGE_SAMPLED_BIT,
	                                                       0,        // no create flags
	                                                       &format_properties);
	return result != VK_ERROR_FORMAT_NOT_SUPPORTED;
}

const VkFormatProperties Device::get_format_properties(VkFormat format) const
{
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
	return format_properties;
}

const Queue &Device::get_queue(uint32_t queue_family_index, uint32_t queue_index)
{
	return queues[queue_family_index][queue_index];
}

const Queue &Device::get_queue_by_flags(VkQueueFlags required_queue_flags, uint32_t queue_index)
{
	for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
	{
		Queue &first_queue = queues[queue_family_index][0];

		VkQueueFlags queue_flags = first_queue.get_properties().queueFlags;
		uint32_t     queue_count = first_queue.get_properties().queueCount;

		if (((queue_flags & required_queue_flags) == required_queue_flags) && queue_index < queue_count)
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

ResourceCache &Device::get_resource_cache()
{
	return resource_cache;
}
}        // namespace vkb
