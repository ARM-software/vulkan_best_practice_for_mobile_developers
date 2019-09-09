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
/**
 * @brief A wrapper class for VkInstance
 *
 * This class is responsible for initializing volk, enumerating over all available extensions and validation layers
 * enabling them if they exist, setting up debug messaging and querying all the physical devices existing on the machine.
 */
class Instance : public NonCopyable
{
  public:
	/**
	 * @brief Initializes the connection to Vulkan
	 * @param application_name The name of the application
	 * @param required_extensions The extensions requested to be enabled
	 * @param required_validation_layers The validation layers to be enabled
	 * @param headless Whether the application is requesting a headless setup or not
	 * @throws runtime_error if the required extensions and validation layers are not found
	 */
	Instance(const std::string &              application_name,
	         const std::vector<const char *> &required_extensions        = {},
	         const std::vector<const char *> &required_validation_layers = {},
	         bool                             headless                   = false);

	~Instance();

	/**
	 * @brief Tries to find the first available discrete GPU
	 * @returns A valid physical device
	 */
	VkPhysicalDevice get_gpu();

	/**
	 * @brief Checks if the given extension is enabled in the VkInstance
	 * @param extension An extension to check
	 */
	bool is_enabled(const char *extension);

	VkInstance get_handle();

	const std::vector<const char *> &get_extensions();

  private:
	/**
	 * @brief The Vulkan instance
	 */
	VkInstance handle{VK_NULL_HANDLE};

	/**
	 * @brief The enabled extensions
	 */
	std::vector<const char *> extensions;

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	/**
	 * @brief The debug report callback
	 */
	VkDebugReportCallbackEXT debug_report_callback{VK_NULL_HANDLE};
#endif

	/**
	 * @brief The physical devices found on the machine
	 */
	std::vector<VkPhysicalDevice> gpus;
};
}        // namespace vkb
