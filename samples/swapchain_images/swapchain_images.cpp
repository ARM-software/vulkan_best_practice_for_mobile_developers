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

#include "swapchain_images.h"

#include "gui.h"
#include "platform/platform.h"
#include "stats.h"

#include "core/device.h"
#include "core/pipeline_layout.h"
#include "core/shader_module.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

#include "scene_graph/components/material.h"
#include "scene_graph/components/pbr_material.h"

#include "gltf_loader.h"

SwapchainImages::SwapchainImages()
{
	get_configuration().insert<vkb::IntSetting>(0, swapchain_image_count, 3);

	get_configuration().insert<vkb::IntSetting>(1, swapchain_image_count, 2);
}

bool SwapchainImages::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	device = std::make_unique<vkb::Device>(get_gpu(0), get_surface(), extensions);

	auto swapchain = std::make_unique<vkb::Swapchain>(*device, get_surface());

	stats = std::make_unique<vkb::Stats>(std::set<vkb::StatIndex>{vkb::StatIndex::frame_times});

	render_context = std::make_unique<vkb::RenderContext>(*device, std::move(swapchain));
	render_context->prepare();

	vkb::ShaderSource vert_shader(vkb::read_binary_file("shaders/base.vert"));
	vkb::ShaderSource frag_shader(vkb::read_binary_file("shaders/base.frag"));

	render_pipeline = std::make_unique<vkb::RenderPipeline>(*render_context, scene, std::move(vert_shader), std::move(frag_shader));

	load_scene("scenes/sponza/Sponza01.gltf");

	auto &camera_node = add_free_camera("main_camera");

	camera = &camera_node.get_component<vkb::sg::Camera>();

	gui = std::make_unique<vkb::Gui>(*render_context, platform.get_dpi_factor());

	return true;
}

void SwapchainImages::update(float delta_time)
{
	// Process GUI input
	if (swapchain_image_count != last_swapchain_image_count)
	{
		render_context->get_device().wait_idle();

		// Create a new swapchain using the old one
		auto new_swapchain = std::make_unique<vkb::Swapchain>(
		    render_context->get_swapchain(),
		    swapchain_image_count);

		render_context->update_swapchain(std::move(new_swapchain));

		last_swapchain_image_count = swapchain_image_count;
	}

	VulkanSample::update(delta_time);
}

void SwapchainImages::draw_gui()
{
	gui->show_options_window(
	    /* body = */ [this]() {
		    ImGui::RadioButton("Double buffering", &swapchain_image_count, 2);
		    ImGui::SameLine();
		    ImGui::RadioButton("Triple buffering", &swapchain_image_count, 3);
		    ImGui::SameLine();
	    },
	    /* lines = */ 1);
}

void SwapchainImages::draw_scene(vkb::CommandBuffer &cmd_buf)
{
	render_pipeline->draw_scene(cmd_buf, *camera);
}

std::unique_ptr<vkb::VulkanSample> create_swapchain_images()
{
	return std::make_unique<SwapchainImages>();
}
