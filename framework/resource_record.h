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

#include "common/vk_common.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/render_pass.h"
#include "core/shader_module.h"

namespace vkb
{
class ResourceCache;

enum class ResourceType
{
	ShaderModule,
	PipelineLayout,
	RenderPass,
	GraphicsPipeline
};

/**
 * @brief Writes Vulkan objects in a memory stream.
 */
class ResourceRecord
{
  public:
	void set_data(const std::vector<uint8_t> &data);

	std::vector<uint8_t> get_data();

	const std::ostringstream &get_stream();

	size_t register_shader_module(VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const std::string &entry_point, const ShaderVariant &shader_variant);

	size_t register_pipeline_layout(const std::vector<ShaderModule *> &shader_modules);

	size_t register_render_pass(const std::vector<Attachment> &   attachments,
	                            const std::vector<LoadStoreInfo> &load_store_infos,
	                            const std::vector<SubpassInfo> &  subpasses);

	size_t register_graphics_pipeline(VkPipelineCache pipeline_cache,
	                                  PipelineState & pipeline_state);

	void set_shader_module(size_t index, const ShaderModule &shader_module);

	void set_pipeline_layout(size_t index, const PipelineLayout &pipeline_layout);

	void set_render_pass(size_t index, const RenderPass &render_pass);

	void set_graphics_pipeline(size_t index, const GraphicsPipeline &graphics_pipeline);

  private:
	std::ostringstream stream;

	std::vector<size_t> shader_module_indices;

	std::vector<size_t> pipeline_layout_indices;

	std::vector<size_t> render_pass_indices;

	std::vector<size_t> graphics_pipeline_indices;

	std::unordered_map<const ShaderModule *, size_t> shader_module_to_index;

	std::unordered_map<const PipelineLayout *, size_t> pipeline_layout_to_index;

	std::unordered_map<const RenderPass *, size_t> render_pass_to_index;

	std::unordered_map<const GraphicsPipeline *, size_t> graphics_pipeline_to_index;
};
}        // namespace vkb
