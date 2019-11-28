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

#include "wait_idle.h"

#include "common/vk_common.h"
#include "gltf_loader.h"
#include "gui.h"
#include "platform/filesystem.h"
#include "platform/platform.h"
#include "rendering/subpasses/forward_subpass.h"
#include "stats.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

WaitIdle::WaitIdle()
{
	auto &config = get_configuration();

	config.insert<vkb::IntSetting>(0, wait_idle_enabled, 0);
	config.insert<vkb::IntSetting>(1, wait_idle_enabled, 1);
}

bool WaitIdle::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	render_context.reset();
	render_context = std::make_unique<CustomRenderContext>(get_device(), get_surface(), platform.get_window().get_width(), platform.get_window().get_height(), wait_idle_enabled);
	prepare_render_context();

	// Load a scene from the assets folder
	load_scene("scenes/bonza/Bonza.gltf");

	// Attach a move script to the camera component in the scene
	auto &camera_node = vkb::add_free_camera(*scene, "main_camera", get_render_context().get_surface_extent());
	camera            = dynamic_cast<vkb::sg::PerspectiveCamera *>(&camera_node.get_component<vkb::sg::Camera>());

	// Example Scene Render Pipeline
	vkb::ShaderSource vert_shader(vkb::fs::read_shader("base.vert"));
	vkb::ShaderSource frag_shader(vkb::fs::read_shader("base.frag"));
	auto              scene_subpass   = std::make_unique<vkb::ForwardSubpass>(get_render_context(), std::move(vert_shader), std::move(frag_shader), *scene, *camera);
	auto              render_pipeline = vkb::RenderPipeline();
	render_pipeline.add_subpass(std::move(scene_subpass));
	set_render_pipeline(std::move(render_pipeline));

	// Add a GUI with the stats you want to monitor
	stats = std::make_unique<vkb::Stats>(std::set<vkb::StatIndex>{vkb::StatIndex::frame_times});
	gui   = std::make_unique<vkb::Gui>(*this, platform.get_window().get_dpi_factor());

	return true;
}

WaitIdle::CustomRenderContext::CustomRenderContext(vkb::Device &device, VkSurfaceKHR surface, uint32_t window_width, uint32_t window_height, int &wait_idle_enabled) :
    RenderContext(device, surface, window_width, window_height),
    wait_idle_enabled(wait_idle_enabled)
{}

void WaitIdle::CustomRenderContext::wait_frame()
{
	// POI
	//
	// If wait idle is enabled, wait using vkDeviceWaitIdle
	//
	// Otherwise, wait using frame's fence with vkWaitForFences

	vkb::RenderFrame &frame = get_active_frame();

	if (wait_idle_enabled == 0)
	{
		get_device().wait_idle();
		frame.reset(false);
	}
	else
	{
		frame.reset(true);
	}
}

void WaitIdle::draw_gui()
{
	bool     landscape = camera->get_aspect_ratio() > 1.0f;
	uint32_t lines     = landscape ? 1 : 2;

	gui->show_options_window(
	    /* body = */ [&]() {
		    ImGui::RadioButton("Wait Idle", &wait_idle_enabled, 0);
		    if (landscape)
		    {
			    ImGui::SameLine();
		    }
		    ImGui::RadioButton("Fences", &wait_idle_enabled, 1);
	    },
	    /* lines = */ lines);
}

std::unique_ptr<vkb::VulkanSample> create_wait_idle()
{
	return std::make_unique<WaitIdle>();
}
