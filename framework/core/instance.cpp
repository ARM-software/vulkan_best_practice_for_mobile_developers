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

#include "instance.h"

#include <algorithm>

namespace vkb
{
namespace
{
#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
                                                     uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
                                                     const char *layer_prefix, const char *message, void * /*user_data*/)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		spdlog::error("[layers:{}]: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		LOGW("[layers:{}]: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		LOGW("[layers:{}]: {}", layer_prefix, message);
	}
	else
	{
		LOGI("[layers:{}]: {}", layer_prefix, message);
	}
	return VK_FALSE;
}
#endif
bool validate_extensions(const std::vector<const char *> &         required,
                         const std::vector<VkExtensionProperties> &available)
{
	for (auto extension : required)
	{
		bool found = false;
		for (auto &available_extension : available)
		{
			if (strcmp(available_extension.extensionName, extension) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			LOGE("Extension {} not found", extension);
			return false;
		}
	}

	return true;
}

bool validate_layers(const std::vector<const char *> &     required,
                     const std::vector<VkLayerProperties> &available)
{
	for (auto layer : required)
	{
		bool found = false;
		for (auto &available_layer : available)
		{
			if (strcmp(available_layer.layerName, layer) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			LOGE("Validation Layer {} not found", layer);
			return false;
		}
	}

	return true;
}
}        // namespace

Instance::Instance(const std::string &              application_name,
                   const std::vector<const char *> &required_extensions,
                   const std::vector<const char *> &required_validation_layers,
                   bool                             headless) :
    extensions{required_extensions}
{
	VkResult result = volkInitialize();
	if (result)
	{
		throw VulkanException(result, "Failed to initialize volk.");
	}

	uint32_t instance_extension_count;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));

	std::vector<VkExtensionProperties> available_instance_extensions(instance_extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, available_instance_extensions.data()));

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	// Try to enable headless surface extension if it exists
	if (headless)
	{
		bool headless_extension = false;
		for (auto &available_extension : available_instance_extensions)
		{
			if (strcmp(available_extension.extensionName, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME) == 0)
			{
				headless_extension = true;
				LOGI("{} is available, enabling it", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
				extensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
			}
		}
		if (!headless_extension)
		{
			LOGW("{} is not available, disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
		}
	}
	else
	{
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	}

	if (!validate_extensions(extensions, available_instance_extensions))
	{
		throw std::runtime_error("Required instance extensions are missing.");
	}

	uint32_t instance_layer_count;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));

	std::vector<VkLayerProperties> instance_layers(instance_layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

	std::vector<const char *> active_instance_layers(required_validation_layers);

#ifdef VKB_VALIDATION_LAYERS
	active_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	if (!validate_layers(active_instance_layers, instance_layers))
	{
		throw std::runtime_error("Required validation layers are missing.");
	}

	VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};

	app_info.pApplicationName   = application_name.c_str();
	app_info.applicationVersion = 0;
	app_info.pEngineName        = "Vulkan Best Practice";
	app_info.engineVersion      = 0;
	app_info.apiVersion         = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instance_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

	instance_info.pApplicationInfo = &app_info;

	instance_info.enabledExtensionCount   = to_u32(extensions.size());
	instance_info.ppEnabledExtensionNames = extensions.data();

	instance_info.enabledLayerCount   = to_u32(active_instance_layers.size());
	instance_info.ppEnabledLayerNames = active_instance_layers.data();

	// Create the Vulkan instance
	result = vkCreateInstance(&instance_info, nullptr, &handle);
	if (result != VK_SUCCESS)
	{
		throw VulkanException(result, "Could not create Vulkan instance");
	}

	volkLoadInstance(handle);

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	VkDebugReportCallbackCreateInfoEXT info = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};

	info.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	info.pfnCallback = debug_callback;

	result = vkCreateDebugReportCallbackEXT(handle, &info, nullptr, &debug_report_callback);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Could not create debug callback.");
	}
#endif

	// Querying valid physical devices on the machine
	uint32_t physical_device_count{0};
	VK_CHECK(vkEnumeratePhysicalDevices(handle, &physical_device_count, nullptr));

	if (physical_device_count < 1)
	{
		throw std::runtime_error("Couldn't find a physical device that supports Vulkan.");
	}

	gpus.resize(physical_device_count);

	VK_CHECK(vkEnumeratePhysicalDevices(handle, &physical_device_count, gpus.data()));
}

Instance::~Instance()
{
#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	vkDestroyDebugReportCallbackEXT(handle, debug_report_callback, nullptr);
#endif

	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyInstance(handle, nullptr);
	}
}

VkPhysicalDevice Instance::get_gpu()
{
	// Find a discrete GPU
	for (auto gpu : gpus)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(gpu, &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			return gpu;
		}
	}

	// Otherwise just pick the first one
	LOGW("Couldn't find a discrete physical device, using integrated graphics");
	return gpus.at(0);
}

bool Instance::is_enabled(const char *extension)
{
	return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

VkInstance Instance::get_handle()
{
	return handle;
}
const std::vector<const char *> &Instance::get_extensions()
{
	return extensions;
}
}        // namespace vkb
