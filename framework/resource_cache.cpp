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

#include "resource_cache.h"

#include <vector>

namespace vkb
{
namespace
{
template <typename T>
inline void hash_param(size_t &seed, const T &value)
{
	hash_combine(seed, value);
}

template <>
inline void hash_param(size_t & /*seed*/, const VkPipelineCache & /*value*/)
{
}

template <>
inline void hash_param<std::vector<uint8_t>>(
    size_t &                    seed,
    const std::vector<uint8_t> &value)
{
	hash_combine(seed, std::string{value.begin(), value.end()});
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
	for (auto &shader_module : value)
	{
		hash_combine(seed, shader_module->get_id());
	}
}

template <>
inline void hash_param<std::vector<ShaderResource>>(
    size_t &                           seed,
    const std::vector<ShaderResource> &value)
{
	for (auto &resource : value)
	{
		hash_combine(seed, resource);
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

template <class T, class... A>
struct RecordHelper
{
	size_t record(ResourceRecord & /*recorder*/, A &... /*args*/)
	{
		return 0;
	}

	void index(ResourceRecord & /*recorder*/, size_t /*index*/, T & /*resource*/)
	{
	}
};

template <class... A>
struct RecordHelper<ShaderModule, A...>
{
	size_t record(ResourceRecord &recorder, A &... args)
	{
		return recorder.register_shader_module(args...);
	}

	void index(ResourceRecord &recorder, size_t index, ShaderModule &shader_module)
	{
		recorder.set_shader_module(index, shader_module);
	}
};

template <class... A>
struct RecordHelper<PipelineLayout, A...>
{
	size_t record(ResourceRecord &recorder, A &... args)
	{
		return recorder.register_pipeline_layout(args...);
	}

	void index(ResourceRecord &recorder, size_t index, PipelineLayout &pipeline_layout)
	{
		recorder.set_pipeline_layout(index, pipeline_layout);
	}
};

template <class... A>
struct RecordHelper<RenderPass, A...>
{
	size_t record(ResourceRecord &recorder, A &... args)
	{
		return recorder.register_render_pass(args...);
	}

	void index(ResourceRecord &recorder, size_t index, RenderPass &render_pass)
	{
		recorder.set_render_pass(index, render_pass);
	}
};

template <class... A>
struct RecordHelper<GraphicsPipeline, A...>
{
	size_t record(ResourceRecord &recorder, A &... args)
	{
		return recorder.register_graphics_pipeline(args...);
	}

	void index(ResourceRecord &recorder, size_t index, GraphicsPipeline &graphics_pipeline)
	{
		recorder.set_graphics_pipeline(index, graphics_pipeline);
	}
};

template <class T, class... A>
T &request_resource(Device &device, ResourceRecord &recorder, std::unordered_map<std::size_t, T> &resources, A &... args)
{
	RecordHelper<T, A...> record_helper;

	std::size_t hash{0U};
	hash_param(hash, args...);

	auto res_it = resources.find(hash);

	if (res_it != resources.end())
	{
		return res_it->second;
	}

	// If we do not have it already, create and cache it
	const char *res_type = typeid(T).name();
	size_t      res_id   = resources.size();

	LOGI("Building #{} cache object ({})", res_id, res_type);

	size_t index = record_helper.record(recorder, args...);

	try
	{
		T resource(device, args...);

		auto res_ins_it = resources.emplace(hash, std::move(resource));

		if (!res_ins_it.second)
		{
			throw std::runtime_error{std::string{"Insertion error for #"} + std::to_string(res_id) + "cache object (" + res_type + ")"};
		}

		res_it = res_ins_it.first;

		record_helper.index(recorder, index, res_it->second);
	}
	catch (const std::exception &e)
	{
		LOGE("Creation error for #{} cache object ({})", res_id, res_type);
		throw e;
	}

	return res_it->second;
}
}        // namespace

ResourceCache::ResourceCache(Device &device) :
    device{device}
{
}

void ResourceCache::warmup(const std::vector<uint8_t> &data)
{
	recorder.set_data(data);

	replayer.play(*this, recorder);
}

std::vector<uint8_t> ResourceCache::serialize()
{
	return recorder.get_data();
}

void ResourceCache::set_pipeline_cache(VkPipelineCache new_pipeline_cache)
{
	pipeline_cache = new_pipeline_cache;
}

ShaderModule &ResourceCache::request_shader_module(VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const ShaderVariant &shader_variant)
{
	std::string entry_point{"main"};
	return request_resource(device, recorder, shader_modules, stage, glsl_source, entry_point, shader_variant);
}

PipelineLayout &ResourceCache::request_pipeline_layout(const std::vector<ShaderModule *> &requested_shader_modules)
{
	return request_resource(device, recorder, pipeline_layouts, requested_shader_modules);
}

DescriptorSetLayout &ResourceCache::request_descriptor_set_layout(const std::vector<ShaderResource> &set_resources)
{
	return request_resource(device, recorder, descriptor_set_layouts, set_resources);
}

GraphicsPipeline &ResourceCache::request_graphics_pipeline(GraphicsPipelineState &graphics_state, const ShaderStageMap<SpecializationInfo> &specialization_infos)
{
	return request_resource(device, recorder, graphics_pipelines, pipeline_cache, graphics_state, specialization_infos);
}

ComputePipeline &ResourceCache::request_compute_pipeline(const PipelineLayout &pipeline_layout, const SpecializationInfo &specialization_info)
{
	return request_resource(device, recorder, compute_pipelines, pipeline_cache, pipeline_layout, specialization_info);
}

DescriptorSet &ResourceCache::request_descriptor_set(DescriptorSetLayout &descriptor_set_layout, const BindingMap<VkDescriptorBufferInfo> &buffer_infos, const BindingMap<VkDescriptorImageInfo> &image_infos)
{
	return request_resource(device, recorder, descriptor_sets, descriptor_set_layout, buffer_infos, image_infos);
}

RenderPass &ResourceCache::request_render_pass(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
{
	return request_resource(device, recorder, render_passes, attachments, load_store_infos, subpasses);
}

Framebuffer &ResourceCache::request_framebuffer(const RenderTarget &render_target, const RenderPass &render_pass)
{
	return request_resource(device, recorder, framebuffers, render_target, render_pass);
}

void ResourceCache::clear_pipelines()
{
	graphics_pipelines.clear();

	compute_pipelines.clear();
}

void ResourceCache::clear_framebuffers()
{
	framebuffers.clear();
}

void ResourceCache::clear()
{
	shader_modules.clear();
	pipeline_layouts.clear();
	descriptor_sets.clear();
	descriptor_set_layouts.clear();
	render_passes.clear();
	clear_pipelines();
	clear_framebuffers();
}
}        // namespace vkb
