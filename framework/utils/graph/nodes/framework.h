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

#include <iostream>
#include <map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <variant>

#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>

#include "core/device.h"
#include "core/image_view.h"
#include "core/shader_module.h"
#include "core/swapchain.h"
#include "fence_pool.h"
#include "rendering/render_context.h"
#include "rendering/render_frame.h"
#include "rendering/render_target.h"
#include "resource_cache.h"
#include "semaphore_pool.h"

namespace vkb
{
namespace utils
{
enum class FrameworkNodeType
{
	Text,
	RenderContext,
	RenderFrame,
	SemaphorePool,
	FencePool,
	CommandPool,
	RenderTarget,
	Swapchain,
	ImageView,
	Image,
	ResourceCache,
	ShaderModule,
	PipelineLayout,
	DescriptorSetLayout,
	RenderPass,
	GraphicsPipeline,
	ComputePipeline,
	DescriptorSet,
	Framebuffer,
	ShaderResource,
	PipelineState,
	SpecializationConstantState,
	VertexInputState,
	InputAssemblyState,
	RasterizationState,
	ViewportState,
	MultisampleState,
	DepthStencilState,
	ColorBlendState,
	ColorBlendAttachmentState,
	VkImage,
	Device
};

/**
 * @brief FrameworkNode is a node type used by utils::Graph to create different node variants for different types of framework components.
 * This structure allows for minimum code cluttering when using the graph api.
 * Note: if you want to add a new framework node definition to the graph it must also be defined here
 */
class FrameworkNode
{
	enum class Group
	{
		Default,
		Core,
		Rendering,
		Framework
	};

  public:
	FrameworkNode() = default;

	FrameworkNode(size_t id, const std::string &text);
	FrameworkNode(size_t id, const std::string &text, size_t owner);
	FrameworkNode(size_t id, const Device &device);
	FrameworkNode(size_t id, const RenderContext &context);
	FrameworkNode(size_t id, const SemaphorePool &semaphore_pool);
	FrameworkNode(size_t id, const FencePool &fence_pool);
	FrameworkNode(size_t id, const RenderFrame &frame, std::string label);
	FrameworkNode(size_t id, const RenderTarget &render_target);
	FrameworkNode(size_t id, const core::ImageView &image_view);
	FrameworkNode(size_t id, const core::Image &image);
	FrameworkNode(size_t id, const Swapchain &swapchain);
	FrameworkNode(size_t id, const ResourceCache &resource_cache);
	FrameworkNode(size_t id, const DescriptorSetLayout &descriptor_set_layouts, size_t hash);
	FrameworkNode(size_t id, const Framebuffer &framebuffers, size_t hash);
	FrameworkNode(size_t id, const RenderPass &render_passes, size_t hash);
	FrameworkNode(size_t id, const RenderPass &render_passes);
	FrameworkNode(size_t id, const ShaderModule &shader_modules);
	FrameworkNode(size_t id, const ShaderResource &shader_resource);
	FrameworkNode(size_t id, const PipelineLayout &pipeline_layouts, size_t hash);
	FrameworkNode(size_t id, const PipelineLayout &pipeline_layouts);
	FrameworkNode(size_t id, const GraphicsPipeline &graphics_pipelines, size_t hash);
	FrameworkNode(size_t id, const ComputePipeline &compute_pipelines, size_t hash);
	FrameworkNode(size_t id, const PipelineState &pipeline_state);
	FrameworkNode(size_t id, const DescriptorSet &descriptor_sets, size_t hash);
	FrameworkNode(size_t id, const SpecializationConstantState &specialization_constant_state);
	FrameworkNode(size_t id, const VertexInputState &vertex_input_state);
	FrameworkNode(size_t id, const InputAssemblyState &input_assembly_state);
	FrameworkNode(size_t id, const RasterizationState &rasterization_state);
	FrameworkNode(size_t id, const ViewportState &viewport_state);
	FrameworkNode(size_t id, const MultisampleState &multisample_state);
	FrameworkNode(size_t id, const DepthStencilState &depth_stencil_state);
	FrameworkNode(size_t id, const ColorBlendState &color_blend_state);
	FrameworkNode(size_t id, const ColorBlendAttachmentState &state);
	FrameworkNode(size_t id, const VkImage &image);

	template <typename T>
	static std::string get_id(FrameworkNodeType type, T value);

	static std::string get_type_str(FrameworkNodeType type);

	nlohmann::json attributes;

  private:
	static std::unordered_map<FrameworkNodeType, std::string> framework_node_type_strings;
};
}        // namespace utils
}        // namespace vkb