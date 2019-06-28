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

#include "resource_record.h"

namespace vkb
{
class ResourceCache;

/**
 * @brief Reads Vulkan objects from a memory stream and creates them in the resource cache.
 */
class ResourceReplay
{
  public:
	ResourceReplay();

	void play(ResourceCache &resource_cache, ResourceRecord &recorder);

  protected:
	void create_shader_module(ResourceCache &resource_cache, std::istringstream &stream);

	void create_pipeline_layout(ResourceCache &resource_cache, std::istringstream &stream);

	void create_render_pass(ResourceCache &resource_cache, std::istringstream &stream);

	void create_graphics_pipeline(ResourceCache &resource_cache, std::istringstream &stream);

  private:
	using ResourceFunc = std::function<void(ResourceCache &, std::istringstream &)>;

	std::unordered_map<ResourceType, ResourceFunc> stream_resources;

	std::vector<ShaderModule *> shader_modules;

	std::vector<PipelineLayout *> pipeline_layouts;

	std::vector<const RenderPass *> render_passes;

	std::vector<const GraphicsPipeline *> graphics_pipelines;
};
}        // namespace vkb
