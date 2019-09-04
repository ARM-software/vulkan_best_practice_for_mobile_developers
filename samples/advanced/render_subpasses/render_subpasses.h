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

#include "rendering/render_pipeline.h"
#include "scene_graph/components/perspective_camera.h"
#include "vulkan_sample.h"

/**
 * @brief The RenderSubpasses sample shows how a significant amount of bandwidth
 *        (L2 cache ext reads and writes) can be saved, by using sub-passes instead
 *        of multiple render passes. In order to highlight the difference, it
 *        implements deferred rendering with and without sub-passes, giving the
 *        user the possibility to change some key settings.
 */
class RenderSubpasses : public vkb::VulkanSample
{
  public:
	RenderSubpasses();

	bool prepare(vkb::Platform &platform) override;

	void update(float delta_time) override;

	virtual ~RenderSubpasses() = default;

	void draw_gui() override;

  private:
	/**
	 * @return A good pipeline
	 */
	std::unique_ptr<vkb::RenderPipeline> create_one_renderpass_two_subpasses();

	/**
	 * @return A geometry render pass which should run first
	 */
	std::unique_ptr<vkb::RenderPipeline> create_geometry_renderpass();

	/**
	 * @return A lighting render pass which should run second
	 */
	std::unique_ptr<vkb::RenderPipeline> create_lighting_renderpass();

	/**
	 * @brief Draws using the good pipeline: one render pass with two sub-passes
	 */
	void draw_render_subpasses(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target);

	/**
	 * @brief Draws using the bad practice: two separate render passes
	 */
	void draw_renderpasses(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target);

	void draw_swapchain_renderpass(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target) override;

	vkb::RenderTarget create_render_target(vkb::core::Image &&swapchain_image);

	/// Good pipeline with two subpasses within one render pass
	std::unique_ptr<vkb::RenderPipeline> render_pipeline{};

	/// 1. Bad pipeline with a geometry subpass in the first render pass
	std::unique_ptr<vkb::RenderPipeline> geometry_render_pipeline{};

	/// 2. Bad pipeline with a lighting subpass in the second render pass
	std::unique_ptr<vkb::RenderPipeline> lighting_render_pipeline{};

	vkb::sg::PerspectiveCamera *camera{};

	/**
	 * @brief Struct that contains configurations for this sample
	 *        with description, options, and current selected value
	 */
	struct Config
	{
		/**
		 * @brief Configurations type
		 */
		enum Type
		{
			RenderTechnique,
			TransientAttachments,
			GBufferSize
		} type;

		/// Used as label by the GUI
		const char *description;

		/// List of options to choose from
		std::vector<const char *> options;

		/// Index of the current selected option
		int value;
	};

	uint16_t last_render_technique{0};
	uint16_t last_transient_attachment{0};
	uint16_t last_g_buffer_size{0};

	VkFormat          albedo_format{VK_FORMAT_R8G8B8A8_UNORM};
	VkFormat          normal_format{VK_FORMAT_A2R10G10B10_UNORM_PACK32};
	VkImageUsageFlags rt_usage_flags{VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT};

	std::vector<Config> configs = {
	    {/* config      = */ Config::RenderTechnique,
	     /* description = */ "Render technique",
	     /* options     = */ {"Subpasses", "Renderpasses"},
	     /* value       = */ 0},
	    {/* config      = */ Config::TransientAttachments,
	     /* description = */ "Transient attachments",
	     /* options     = */ {"Enabled", "Disabled"},
	     /* value       = */ 0},
	    {/* config      = */ Config::GBufferSize,
	     /* description = */ "G-Buffer size",
	     /* options     = */ {"128-bit", "More"},
	     /* value       = */ 0}};
};

std::unique_ptr<vkb::VulkanSample> create_render_subpasses();
