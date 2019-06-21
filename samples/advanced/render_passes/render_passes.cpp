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

#include "render_passes.h"

#include "platform/platform.h"

#include "common.h"
#include "gltf_loader.h"
#include "gui.h"
#include "platform/file.h"
#include "stats.h"

#include "rendering/subpasses/scene_subpass.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

RenderPassesSample::RenderPassesSample()
{
	auto &config = get_configuration();

	config.insert<vkb::IntSetting>(0, load.value, 0);
	config.insert<vkb::IntSetting>(0, store.value, 0);

	config.insert<vkb::IntSetting>(1, load.value, 1);
	config.insert<vkb::IntSetting>(1, store.value, 1);
}

void RenderPassesSample::reset_stats_view()
{
	if (load.value == VK_ATTACHMENT_LOAD_OP_LOAD)
	{
		gui->get_stats_view().reset_max_value(vkb::StatIndex::l2_ext_read_bytes);
	}

	if (store.value == VK_ATTACHMENT_STORE_OP_STORE)
	{
		gui->get_stats_view().reset_max_value(vkb::StatIndex::l2_ext_write_bytes);
	}
}

void RenderPassesSample::draw_gui()
{
	auto lines = radio_buttons.size();
	if (camera->get_aspect_ratio() < 1.0f)
	{
		// In portrait, show buttons below heading
		lines = lines * 2;
	}

	gui->show_options_window(
	    /* body = */ [this, lines]() {
		    // For every option set
		    for (size_t i = 0; i < radio_buttons.size(); ++i)
		    {
			    // Avoid conflicts between buttons with identical labels
			    ImGui::PushID(vkb::to_u32(i));

			    auto &radio_button = radio_buttons[i];

			    ImGui::Text("%s: ", radio_button->description);

			    if (camera->get_aspect_ratio() > 1.0f)
			    {
				    // In landscape, show all options following the heading
				    ImGui::SameLine();
			    }

			    // For every option
			    for (size_t j = 0; j < radio_button->options.size(); ++j)
			    {
				    ImGui::RadioButton(radio_button->options[j], &radio_button->value, vkb::to_u32(j));

				    if (j < radio_button->options.size() - 1)
				    {
					    ImGui::SameLine();
				    }
			    }

			    ImGui::PopID();
		    }
	    },
	    /* lines = */ vkb::to_u32(lines));
}

bool RenderPassesSample::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	device = std::make_unique<vkb::Device>(get_gpu(0), get_surface(), extensions);

	auto enabled_stats = {vkb::StatIndex::l2_ext_read_bytes, vkb::StatIndex::l2_ext_write_bytes};

	stats = std::make_unique<vkb::Stats>(enabled_stats);

	auto swapchain = std::make_unique<vkb::Swapchain>(*device, get_surface());

	render_context = std::make_unique<vkb::RenderContext>(std::move(swapchain));

	load_scene("scenes/sponza/Sponza01.gltf");
	auto &camera_node = add_free_camera("main_camera");
	camera            = dynamic_cast<vkb::sg::PerspectiveCamera *>(&camera_node.get_component<vkb::sg::Camera>());

	vkb::ShaderSource vert_shader(vkb::file::read_asset("shaders/base.vert"));
	vkb::ShaderSource frag_shader(vkb::file::read_asset("shaders/base.frag"));
	auto              scene_subpass = std::make_unique<vkb::SceneSubpass>(*render_context, std::move(vert_shader), std::move(frag_shader), scene, *camera);

	render_pipeline = std::make_unique<vkb::RenderPipeline>();
	render_pipeline->add_subpass(std::move(scene_subpass));

	gui = std::make_unique<vkb::Gui>(*render_context, platform.get_dpi_factor());

	return true;
}

void RenderPassesSample::draw_swapchain_renderpass(vkb::CommandBuffer &command_buffer, vkb::RenderTarget &render_target)
{
	std::vector<vkb::LoadStoreInfo> load_store{2};

	// The load operation for the color attachment is selected by the user at run-time
	auto loadop            = static_cast<VkAttachmentLoadOp>(load.value);
	load_store[0].load_op  = loadop;
	load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	load_store[1].load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Store operation for depth attachment is selected by the user at run-time
	load_store[1].store_op = static_cast<VkAttachmentStoreOp>(store.value);

	std::vector<VkClearValue> clear_value{2};
	clear_value[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clear_value[1].depthStencil = {1.0f, ~0U};

	command_buffer.begin_render_pass(render_target, load_store, clear_value);

	vkb::ColorBlendState blend_state{};
	blend_state.attachments = {vkb::ColorBlendAttachmentState{}};
	command_buffer.set_color_blend_state(blend_state);

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

	draw_scene(command_buffer);

	gui->draw(command_buffer);

	command_buffer.end_render_pass();
}

void RenderPassesSample::draw_scene(vkb::CommandBuffer &command_buffer)
{
	render_pipeline->draw(command_buffer);
}

void RenderPassesSample::update(float delta_time)
{
	VulkanSample::update(delta_time);

	// Use an exponential moving average to smooth values
	const float alpha = 0.01f;
	frame_rate        = (1.0f / delta_time) * alpha + frame_rate * (1.0f - alpha);
}

std::unique_ptr<vkb::VulkanSample> create_render_passes()
{
	return std::make_unique<RenderPassesSample>();
}
