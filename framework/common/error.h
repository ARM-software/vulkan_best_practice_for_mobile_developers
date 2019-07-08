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

#include <cassert>
#include <stdexcept>
#include <string>

#include "logging.h"
#include "vk_common.h"

#if defined(__clang__)
// CLANG ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS()                             \
		_Pragma("clang diagnostic push")                        \
		    _Pragma("clang diagnostic ignored \"-Wall\"")       \
		        _Pragma("clang diagnostic ignored \"-Wextra\"") \
		            _Pragma("clang diagnostic ignored \"-Wtautological-compare\"")

#	define VKBP_ENABLE_WARNINGS() \
		_Pragma("clang diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
// GCC ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS()                             \
		_Pragma("GCC diagnostic push")                          \
		    _Pragma("GCC diagnostic ignored \"-Wall\"")         \
		        _Pragma("clang diagnostic ignored \"-Wextra\"") \
		            _Pragma("clang diagnostic ignored \"-Wtautological-compare\"")

#	define VKBP_ENABLE_WARNINGS() \
		_Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
// MSVC ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS() \
		__pragma(warning(push, 0))

#	define VKBP_ENABLE_WARNINGS() \
		__pragma(warning(pop))
#endif

namespace vkb
{
/**
 * @brief Vulkan exception structure
 */
class VulkanException : public std::runtime_error
{
  public:
	/**
	 * @brief Vulkan exception constructor
	 */
	VulkanException(VkResult result, const std::string &msg = "Vulkan error");

	/**
	 * @brief Returns the Vulkan error code as string
	 * @return String message of exception
	 */
	const char *what() const noexcept override;

  private:
	std::string error_message;
};
}        // namespace vkb

/// @brief Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x)                                                                   \
	do                                                                                \
	{                                                                                 \
		VkResult err = x;                                                             \
		if (err)                                                                      \
		{                                                                             \
			LOGE("Detected Vulkan error {} at {}:{}.", int(err), __FILE__, __LINE__); \
			abort();                                                                  \
		}                                                                             \
	} while (0)

#define ASSERT_VK_HANDLE(handle)                                  \
	do                                                            \
	{                                                             \
		if ((handle) == VK_NULL_HANDLE)                           \
		{                                                         \
			LOGE("Handle is NULL at {}:{}.", __FILE__, __LINE__); \
			abort();                                              \
		}                                                         \
	} while (0)

#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG)
#	define VKB_DEBUG
#endif
