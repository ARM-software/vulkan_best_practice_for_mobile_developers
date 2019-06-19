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

#define VKBP_TAG "VulkanBestPractice"

#if defined(__clang__)
// CLANG ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS        \
		_Pragma("clang diagnostic push") \
		    _Pragma("clang diagnostic ignored \"-Wall\"")

#	define VKBP_ENABLE_WARNINGS \
		_Pragma("clang diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
// GCC ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS      \
		_Pragma("GCC diagnostic push") \
		    _Pragma("GCC diagnostic ignored \"-Wall\"")

#	define VKBP_ENABLE_WARNINGS \
		_Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
// MSVC ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS \
		__pragma(warning(push, 0))

#	define VKBP_ENABLE_WARNINGS \
		__pragma(warning(pop))
#endif

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <vk_mem_alloc.h>
#include <volk.h>

VKBP_DISABLE_WARNINGS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
VKBP_ENABLE_WARNINGS

#include <spdlog/spdlog.h>

#define LOGGER_FORMAT "[%^%l%$] %v"
#define PROJECT_NAME "VulkanBestPractice"

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error(__VA_ARGS__);

template <class T>
using ShaderStageMap = std::map<VkShaderStageFlagBits, T>;

template <class T>
using BindingMap = std::unordered_map<uint32_t, std::map<uint32_t, T>>;

namespace vkb
{
template <typename T>
inline void read(std::istringstream &is, T &value)
{
	is.read(reinterpret_cast<char *>(&value), sizeof(T));
}

inline void read(std::istringstream &is, std::string &value)
{
	std::size_t size;
	read(is, size);
	value.resize(size);
	is.read(value.data(), size);
}

template <class T>
inline void read(std::istringstream &is, std::set<T> &value)
{
	std::size_t size;
	read(is, size);
	for (uint32_t i = 0; i < size; i++)
	{
		T item;
		is.read(reinterpret_cast<char *>(&item), sizeof(T));
		value.insert(std::move(item));
	}
}

template <class T>
inline void read(std::istringstream &is, std::vector<T> &value)
{
	std::size_t size;
	read(is, size);
	value.resize(size);
	is.read(reinterpret_cast<char *>(value.data()), value.size() * sizeof(T));
}

template <class T, uint32_t N>
inline void read(std::istringstream &is, std::array<T, N> &value)
{
	is.read(reinterpret_cast<char *>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void read(std::istringstream &is, T &first_arg, Args &... args)
{
	read(is, first_arg);

	read(is, args...);
}

template <typename T>
inline void write(std::ostringstream &os, const T &value)
{
	os.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

inline void write(std::ostringstream &os, const std::string &value)
{
	write(os, value.size());
	os.write(value.data(), value.size());
}

template <class T>
inline void write(std::ostringstream &os, const std::set<T> &value)
{
	write(os, value.size());
	for (const T &item : value)
	{
		os.write(reinterpret_cast<const char *>(&item), sizeof(T));
	}
}

template <class T>
inline void write(std::ostringstream &os, const std::vector<T> &value)
{
	write(os, value.size());
	os.write(reinterpret_cast<const char *>(value.data()), value.size() * sizeof(T));
}

template <class T, uint32_t N>
inline void write(std::ostringstream &os, const std::array<T, N> &value)
{
	os.write(reinterpret_cast<const char *>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void write(std::ostringstream &os, const T &first_arg, const Args &... args)
{
	write(os, first_arg);

	write(os, args...);
}

/**
 * @brief Helper function to combine a given hash
 *        with a generated hash for the input param.
 */
template <class T>
inline void hash_combine(size_t &seed, const T &v)
{
	std::hash<T> hasher;
	glm::detail::hash_combine(seed, hasher(v));
}

/**
 * @brief Helper function to convert a data type
 *        to string using output stream operator.
 * @param value The object to be converted to string
 * @return String version of the given object
 */
template <class T>
inline std::string to_string(const T &value)
{
	std::stringstream ss;
	ss << std::fixed << value;
	return ss.str();
}

/**
 * @brief Helper function to check size_t is correctly converted to uint32_t
 * @param value Value of type @ref size_t to convert
 * @return An @ref uint32_t representation of the same value
 */
uint32_t to_u32(const size_t value);

/**
 * @brief Helper function to determine if a Vulkan format is depth only.
 * @param format Vulkan format to check.
 * @return True if format is a depth only, false otherwise.
 */
bool is_depth_only_format(VkFormat format);

/**
 * @brief Helper function to determine if a Vulkan format is depth or stencil.
 * @param format Vulkan format to check.
 * @return True if format is a depth or stencil, false otherwise.
 */
bool is_depth_stencil_format(VkFormat format);

/**
 * @brief Helper function to determine if a Vulkan descriptor type is a dynamic storage buffer or dynamic uniform buffer.
 * @param descriptor_type Vulkan descriptor type to check.
 * @return True if type is dynamic buffer, false otherwise.
 */
bool is_dynamic_buffer_descriptor_type(VkDescriptorType descriptor_type);

/**
 * @brief Helper function to determine if a Vulkan descriptor type is a buffer (either uniform or storage buffer, dynamic or not).
 * @param descriptor_type Vulkan descriptor type to check.
 * @return True if type is buffer, false otherwise.
 */
bool is_buffer_descriptor_type(VkDescriptorType descriptor_type);

/**
 * @brief Helper function to get the bits per pixel of a Vulkan format.
 * @param format Vulkan format to check.
 * @return The bits per pixel of the given format, -1 for invalid formats.
 */
int32_t get_bits_per_pixel(VkFormat format);

/**
 * @brief Helper function to convert a VkFormat enum to a string
 * @param format Vulkan format to convert.
 * @return The string to return.
 */
const std::string convert_format_to_string(VkFormat format);
}        // namespace vkb

namespace vkb
{
/**
 * @brief Image memory barrier structure used to define
 *        memory access for an image view during command recording.
 */
struct ImageMemoryBarrier
{
	VkPipelineStageFlags src_stage_mask{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};

	VkPipelineStageFlags dst_stage_mask{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

	VkAccessFlags src_access_mask{0};

	VkAccessFlags dst_access_mask{0};

	VkImageLayout old_layout{VK_IMAGE_LAYOUT_UNDEFINED};

	VkImageLayout new_layout{VK_IMAGE_LAYOUT_UNDEFINED};
};

/**
 * @brief Helper class to disable the copy constructor and copy 
 *        assignment operator of any inherited a class to be non copyable.
 */
class NonCopyable
{
  public:
	/**
	 * @brief Constructor
	 */
	NonCopyable() = default;

	/**
	 * @brief Destructor
	 */
	~NonCopyable() = default;

	/**
	 * @brief Move constructor
	 */
	NonCopyable(NonCopyable &&) = default;

	/**
	 * @brief Move assignment operator
	 */
	NonCopyable &operator=(NonCopyable &&) noexcept = default;

  private:
	/**
	 * @brief Deleted copy constructor
	 */
	NonCopyable(const NonCopyable &) = delete;

	/**
	 * @brief Deleted copy assignment operator
	 */
	NonCopyable &operator=(const NonCopyable &) = delete;
};

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

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include <android/native_window_jni.h>
#	include <android_native_app_glue.h>
#else
#	if defined(VK_USE_PLATFORM_WIN32_KHR)
#		include <cstdio>
#		define NOMINMAX
#		include <Windows.h>
#	endif
#	include <GLFW/glfw3.h>
#endif

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
