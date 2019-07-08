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

#include "afbc.h"

#include "common/vk_common.h"
#include "gltf_loader.h"
#include "gui.h"
#include "platform/file.h"
#include "platform/platform.h"
#include "rendering/subpasses/scene_subpass.h"
#include "stats.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

AFBCSample::AFBCSample()
{
	get_configuration().insert<vkb::BoolSetting>(0, afbc_enabled, false);

	get_configuration().insert<vkb::BoolSetting>(1, afbc_enabled, true);
}

bool AFBCSample::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	device = std::make_unique<vkb::Device>(get_gpu(), get_surface(), extensions);

	auto swapchain = std::make_unique<vkb::Swapchain>(*device,
	                                                  get_surface(),
	                                                  VkExtent2D({}),
	                                                  3,
	                                                  VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	                                                  VK_PRESENT_MODE_FIFO_KHR,
	                                                  /* We want AFBC disabled by default, hence we create swapchain with VK_IMAGE_USAGE_STORAGE_BIT. */
	                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

	stats = std::make_unique<vkb::Stats>(std::set<vkb::StatIndex>{vkb::StatIndex::l2_ext_write_bytes});

	render_context = std::make_unique<vkb::RenderContext>(std::move(swapchain));

	load_scene("scenes/sponza/Sponza01.gltf");
	auto &camera_node = add_free_camera("main_camera");
	camera            = &camera_node.get_component<vkb::sg::Camera>();

	vkb::ShaderSource vert_shader(vkb::file::read_asset("shaders/base.vert"));
	vkb::ShaderSource frag_shader(vkb::file::read_asset("shaders/base.frag"));
	auto              scene_subpass = std::make_unique<vkb::SceneSubpass>(*render_context, std::move(vert_shader), std::move(frag_shader), *scene, *camera);

	render_pipeline = std::make_unique<vkb::RenderPipeline>();
	render_pipeline->add_subpass(std::move(scene_subpass));

	gui = std::make_unique<vkb::Gui>(*render_context, platform.get_dpi_factor());

	return true;
}

void AFBCSample::update(float delta_time)
{
	if (afbc_enabled != afbc_enabled_last_value)
	{
		std::set<VkImageUsageFlagBits> image_usage_flags = {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

		if (!afbc_enabled)
		{
			image_usage_flags.insert(VK_IMAGE_USAGE_STORAGE_BIT);
		}

		render_context->get_device().wait_idle();

		auto new_swapchain = std::make_unique<vkb::Swapchain>(
		    render_context->get_swapchain(),
		    image_usage_flags);

		render_context->update_swapchain(std::move(new_swapchain));

		afbc_enabled_last_value = afbc_enabled;
	}

	VulkanSample::update(delta_time);
}

void AFBCSample::draw_gui()
{
	gui->show_options_window(
	    /* body = */ [this]() {
		    ImGui::Checkbox("AFBC", &afbc_enabled);
	    },
	    /* lines = */ 1);
}

void AFBCSample::draw_scene(vkb::CommandBuffer &cmd_buf)
{
	render_pipeline->draw(cmd_buf);
}

std::unique_ptr<vkb::VulkanSample> create_afbc()
{
	return std::make_unique<AFBCSample>();
}
