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

#include <vector>

namespace vkb
{
namespace detail
{
template <typename T>
inline void hash_param(size_t &seed, const T &value)
{
	hash_combine(seed, value);
}

template <>
inline void hash_param<std::vector<Attachment>>(
    size_t &                       seed,
    const std::vector<Attachment> &value)
{
	for (auto &attachment : value)
	{
		hash_combine(seed, attachment);
	}
}

template <>
inline void hash_param<std::vector<LoadStoreInfo>>(
    size_t &                          seed,
    const std::vector<LoadStoreInfo> &value)
{
	for (auto &load_store_info : value)
	{
		hash_combine(seed, load_store_info);
	}
}

template <>
inline void hash_param<std::vector<SubpassInfo>>(
    size_t &                        seed,
    const std::vector<SubpassInfo> &value)
{
	for (auto &subpass_info : value)
	{
		hash_combine(seed, subpass_info);
	}
}

template <>
inline void hash_param<std::vector<ShaderModule *>>(
    size_t &                           seed,
    const std::vector<ShaderModule *> &value)
{
	for (auto shader_module : value)
	{
		hash_combine(seed, *shader_module);
	}
}

template <>
inline void hash_param<std::unordered_map<uint32_t, std::map<uint32_t, VkDescriptorBufferInfo>>>(
    size_t &                                                                        seed,
    const std::unordered_map<uint32_t, std::map<uint32_t, VkDescriptorBufferInfo>> &value)
{
	for (auto &binding_set : value)
	{
		hash_combine(seed, binding_set.first);

		for (auto &binding_element : binding_set.second)
		{
			hash_combine(seed, binding_element.first);
			hash_combine(seed, binding_element.second);
		}
	}
}

template <>
inline void hash_param<std::unordered_map<uint32_t, std::map<uint32_t, VkDescriptorImageInfo>>>(
    size_t &                                                                       seed,
    const std::unordered_map<uint32_t, std::map<uint32_t, VkDescriptorImageInfo>> &value)
{
	for (auto &binding_set : value)
	{
		hash_combine(seed, binding_set.first);

		for (auto &binding_element : binding_set.second)
		{
			hash_combine(seed, binding_element.first);
			hash_combine(seed, binding_element.second);
		}
	}
}

template <>
inline void hash_param<std::map<VkShaderStageFlagBits, SpecializationInfo>>(
    size_t &                                                   seed,
    const std::map<VkShaderStageFlagBits, SpecializationInfo> &value)
{
	for (auto &stage_specialization : value)
	{
		hash_combine(seed, static_cast<std::underlying_type<VkShaderStageFlagBits>::type>(stage_specialization.first));
		hash_combine(seed, stage_specialization.second);
	}
}

template <typename T, typename... Args>
inline void hash_param(size_t &seed, const T &first_arg, const Args &... args)
{
	hash_param(seed, first_arg);

	hash_param(seed, args...);
}
}        // namespace detail

template <typename T>
template <typename... Args>
inline T &CacheResource<T>::request_resource(Args &&... args)
{
	size_t res_hash = 0;

	detail::hash_param(res_hash, args...);

	auto res_it = cache_resources.find(res_hash);

	if (res_it != cache_resources.end())
	{
		return res_it->second;
	}

	// If we do not have it already, create and cache it
	const char *res_type = typeid(T).name();
	size_t      res_id   = cache_resources.size();

	LOGI("Building #{} cache object ({})", res_id, res_type);

	try
	{
		T resource(std::forward<Args>(args)...);

		auto res_ins_it = cache_resources.emplace(res_hash, std::move(resource));

		if (!res_ins_it.second)
		{
			throw std::exception();
		}

		return res_ins_it.first->second;
	}
	catch (const std::exception &e)
	{
		LOGE("Creation error for #{} cache object ( {} )", res_id, res_type);
		throw e;
	}
}

template <typename T>
inline void vkb::CacheResource<T>::clear()
{
	cache_resources.clear();
}
}        // namespace vkb
