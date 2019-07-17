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

#include "render_subpasses.h"

#include "platform/platform.h"
#include "rendering/pipeline_state.h"
#include "rendering/render_context.h"
#include "rendering/render_pipeline.h"
#include "rendering/subpasses/lighting_subpass.h"
#include "rendering/subpasses/scene_subpass.h"
#include "scene_graph/node.h"

RenderSubpasses::RenderSubpasses()
{
	auto &config = get_configuration();

	// Good settings
	config.insert<vkb::IntSetting>(0, configs[Config::RenderTechnique].value, 0);
	config.insert<vkb::IntSetting>(0, configs[Config::TransientAttachments].value, 0);
	config.insert<vkb::IntSetting>(0, configs[Config::GBufferSize].value, 0);

	// Use two render passes
	config.insert<vkb::IntSetting>(1, configs[Config::RenderTechnique].value, 1);
	config.insert<vkb::IntSetting>(1, configs[Config::TransientAttachments].value, 0);
	config.insert<vkb::IntSetting>(1, configs[Config::GBufferSize].value, 0);

	// Disable transient attachments
	config.insert<vkb::IntSetting>(2, configs[Config::RenderTechnique].value, 0);
	config.insert<vkb::IntSetting>(2, configs[Config::TransientAttachments].value, 1);
	config.insert<vkb::IntSetting>(2, configs[Config::GBufferSize].value, 0);

	// Increase G-buffer size
	config.insert<vkb::IntSetting>(3, configs[Config::RenderTechnique].value, 0);
	config.insert<vkb::IntSetting>(3, configs[Config::TransientAttachments].value, 0);
	config.insert<vkb::IntSetting>(3, configs[Config::GBufferSize].value, 1);
}

vkb::RenderTarget RenderSubpasses::create_render_target(vkb::core::Image &&swapchain_image)
{
	auto &device = swapchain_image.get_device();
	auto &extent = swapchain_image.get_extent();

	// G-Buffer should fit 128-bit budget for buffer color storage
	// in order to enable subpasses merging by the driver
	// Light (swapchain_image) RGBA8_UNORM   (32-bit)
	// Albedo                  RGBA8_UNORM   (32-bit)
	// Normal                  RGB10A2_UNORM (32-bit)

	VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	if (configs[Config::TransientAttachments].value == 0)
	{
		usage_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	}
	else
	{
		LOGI("Creating non transient attachments");
	}

	vkb::core::Image depth_image{device,
	                             extent,
	                             VK_FORMAT_D32_SFLOAT,
	                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | usage_flags,
	                             VMA_MEMORY_USAGE_GPU_ONLY};

	vkb::core::Image albedo_image{device,
	                              extent,
	                              configs[Config::GBufferSize].value == 0 ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R16G16B16A16_UNORM,
	                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | usage_flags,
	                              VMA_MEMORY_USAGE_GPU_ONLY};

	vkb::core::Image normal_image{device,
	                              extent,
	                              configs[Config::GBufferSize].value == 0 ? VK_FORMAT_A2R10G10B10_UNORM_PACK32 : VK_FORMAT_R16G16B16A16_UNORM,
	                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | usage_flags,
	                              VMA_MEMORY_USAGE_GPU_ONLY};

	std::vector<vkb::core::Image> images;

	// Attachment 0
	images.push_back(std::move(swapchain_image));

	// Attachment 1
	images.push_back(std::move(depth_image));

	// Attachment 2
	images.push_back(std::move(albedo_image));

	// Attachment 3
	images.push_back(std::move(normal_image));

	return vkb::RenderTarget{std::move(images)};
}

bool RenderSubpasses::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	device = std::make_unique<vkb::Device>(get_gpu(), get_surface(), extensions);

	VkImageUsageFlags swapchain_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	auto              swapchain       = std::make_unique<vkb::Swapchain>(*device, get_surface(), VkExtent2D{}, 3, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, VK_PRESENT_MODE_FIFO_KHR, swapchain_usage);

	render_context = std::make_unique<vkb::RenderContext>(std::move(swapchain), std::bind(&RenderSubpasses::create_render_target, this, std::placeholders::_1));

	load_scene("scenes/sponza/Sponza01.gltf");

	auto &camera_node = add_free_camera("main_camera");
	camera            = dynamic_cast<vkb::sg::PerspectiveCamera *>(&camera_node.get_component<vkb::sg::Camera>());

	render_pipeline = create_one_renderpass_two_subpasses();

	geometry_render_pipeline = create_geometry_renderpass();
	lighting_render_pipeline = create_lighting_renderpass();

	// Enable gui
	gui = std::make_unique<vkb::Gui>(*render_context, platform.get_dpi_factor());

	// Enable stats
	auto enabled_stats = {vkb::StatIndex::fragment_jobs,
	                      vkb::StatIndex::tiles,
	                      vkb::StatIndex::l2_ext_read_bytes,
	                      vkb::StatIndex::l2_ext_write_bytes};
	stats              = std::make_unique<vkb::Stats>(enabled_stats);

	return true;
}

void RenderSubpasses::draw_gui()
{
	auto lines = configs.size();
	if (camera->get_aspect_ratio() < 1.0f)
	{
		// In portrait, show buttons below heading
		lines = lines * 2;
	}

	gui->show_options_window(
	    /* body = */ [this, lines]() {
		    // Create a line for every config
		    for (size_t i = 0; i < configs.size(); ++i)
		    {
			    // Avoid conflicts between buttons with identical labels
			    ImGui::PushID(vkb::to_u32(i));

			    auto &config = configs[i];

			    ImGui::Text("%s: ", config.description);

			    if (camera->get_aspect_ratio() > 1.0f)
			    {
				    // In landscape, show all options following the heading
				    ImGui::SameLine();
			    }

			    // Create a radio button for every option
			    for (size_t j = 0; j < config.options.size(); ++j)
			    {
				    if (ImGui::RadioButton(config.options[j], &config.value, vkb::to_u32(j)))
				    {
					    if (config.type == Config::TransientAttachments ||
					        config.type == Config::GBufferSize)
					    {
						    LOGI("Recreating render target");
						    render_context->update_swapchain(std::make_unique<vkb::Swapchain>(std::move(render_context->get_swapchain())));
					    }
				    }

				    // Keep it on the same line til the last one
				    if (j < config.options.size() - 1)
				    {
					    ImGui::SameLine();
				    }
			    }

			    ImGui::PopID();
		    }
	    },
	    /* lines = */ vkb::to_u32(lines));
}

/**
 * @return Load store info to clear all and store only the swapchain
 */
std::vector<vkb::LoadStoreInfo> get_clear_all_store_swapchain()
{
	// Clear every attachment and store only swapchain
	std::vector<vkb::LoadStoreInfo> load_store{4};

	// Swapchain
	load_store[0].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	// Depth
	load_store[1].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[1].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Albedo
	load_store[2].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[2].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Normal
	load_store[3].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[3].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	return load_store;
}

/**
 * @return Clear values common to all pipelines
 */
std::vector<VkClearValue> get_clear_value()
{
	// Clear values
	std::vector<VkClearValue> clear_value{4};
	clear_value[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clear_value[1].depthStencil = {1.0f, ~0U};
	clear_value[2].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clear_value[3].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};

	return clear_value;
}

std::unique_ptr<vkb::RenderPipeline> RenderSubpasses::create_one_renderpass_two_subpasses()
{
	// Geometry subpass
	auto geometry_vs   = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/geometry.vert")};
	auto geometry_fs   = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/geometry.frag")};
	auto scene_subpass = std::make_unique<vkb::SceneSubpass>(*render_context, std::move(geometry_vs), std::move(geometry_fs), *scene, *camera);

	// Outputs are depth, albedo, and normal
	scene_subpass->set_output_attachments({1, 2, 3});

	// Lighting subpass
	auto lighting_vs      = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/lighting.vert")};
	auto lighting_fs      = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/lighting.frag")};
	auto lighting_subpass = std::make_unique<vkb::LightingSubpass>(*render_context, std::move(lighting_vs), std::move(lighting_fs), *camera);

	// Inputs are depth, albedo, and normal from the geometry subpass
	lighting_subpass->set_input_attachments({1, 2, 3});

	// Create subpasses pipeline
	std::vector<std::unique_ptr<vkb::Subpass>> subpasses{};
	subpasses.push_back(std::move(scene_subpass));
	subpasses.push_back(std::move(lighting_subpass));

	auto render_pipeline = std::make_unique<vkb::RenderPipeline>(std::move(subpasses));

	render_pipeline->set_load_store(get_clear_all_store_swapchain());

	render_pipeline->set_clear_value(get_clear_value());

	return render_pipeline;
}

/**
 * @return Load store info to clear and store only the swapchain
 */
std::vector<vkb::LoadStoreInfo> get_clear_store_all()
{
	// Clear and store every attachment
	std::vector<vkb::LoadStoreInfo> load_store{4};

	// Swapchain
	load_store[0].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	// Depth
	load_store[1].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[1].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	// Albedo
	load_store[2].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[2].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	// Normal
	load_store[3].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[3].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	return load_store;
}

std::unique_ptr<vkb::RenderPipeline> RenderSubpasses::create_geometry_renderpass()
{
	// Geometry subpass
	auto geometry_vs   = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/geometry.vert")};
	auto geometry_fs   = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/geometry.frag")};
	auto scene_subpass = std::make_unique<vkb::SceneSubpass>(*render_context, std::move(geometry_vs), std::move(geometry_fs), *scene, *camera);

	// Outputs are depth, albedo, and normal
	scene_subpass->set_output_attachments({1, 2, 3});

	// Create geomtry pipeline
	std::vector<std::unique_ptr<vkb::Subpass>> scene_subpasses{};
	scene_subpasses.push_back(std::move(scene_subpass));

	auto geometry_render_pipeline = std::make_unique<vkb::RenderPipeline>(std::move(scene_subpasses));

	geometry_render_pipeline->set_load_store(get_clear_store_all());

	geometry_render_pipeline->set_clear_value(get_clear_value());

	return geometry_render_pipeline;
}

std::unique_ptr<vkb::RenderPipeline> RenderSubpasses::create_lighting_renderpass()
{
	// Lighting subpass
	auto lighting_vs      = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/lighting.vert")};
	auto lighting_fs      = vkb::ShaderSource{vkb::fs::read_asset("shaders/deferred/lighting.frag")};
	auto lighting_subpass = std::make_unique<vkb::LightingSubpass>(*render_context, std::move(lighting_vs), std::move(lighting_fs), *camera);

	// Inputs are depth, albedo, and normal from the geometry subpass
	lighting_subpass->set_input_attachments({1, 2, 3});
	// Create lighting pipeline
	std::vector<std::unique_ptr<vkb::Subpass>> lighting_subpasses{};
	lighting_subpasses.push_back(std::move(lighting_subpass));

	auto lighting_render_pipeline = std::make_unique<vkb::RenderPipeline>(std::move(lighting_subpasses));

	lighting_render_pipeline->set_load_store(get_clear_all_store_swapchain());

	lighting_render_pipeline->set_clear_value(get_clear_value());

	return lighting_render_pipeline;
}

void draw_pipeline(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target, vkb::RenderPipeline &render_pipeline, vkb::Gui *gui = nullptr)
{
	auto &extent = render_target.get_extent();

	VkViewport viewport{};
	viewport.width    = static_cast<float>(extent.width);
	viewport.height   = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	command_buffer.set_viewport(0, {viewport});

	VkRect2D scissor{};
	scissor.extent = extent;
	command_buffer.set_scissor(0, {scissor});

	render_pipeline.draw(command_buffer, render_target);

	if (gui)
	{
		gui->draw(command_buffer);
	}

	command_buffer.resolve_subpasses();
	command_buffer.end_render_pass();
}

void RenderSubpasses::draw_render_subpasses(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target)
{
	draw_pipeline(command_buffer, render_target, *render_pipeline, gui.get());
}

void RenderSubpasses::draw_renderpasses(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target)
{
	// First render pass (no gui)
	draw_pipeline(command_buffer, render_target, *geometry_render_pipeline);

	// Second render pass
	draw_pipeline(command_buffer, render_target, *lighting_render_pipeline, gui.get());
}

void RenderSubpasses::draw_swapchain_renderpass(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target)
{
	if (configs[Config::RenderTechnique].value == 0)
	{
		// Efficient way
		draw_render_subpasses(command_buffer, render_target);
	}
	else
	{
		// Inefficient way
		draw_renderpasses(command_buffer, render_target);
	}
}

std::unique_ptr<vkb::VulkanSample> create_render_subpasses()
{
	return std::make_unique<RenderSubpasses>();
}
