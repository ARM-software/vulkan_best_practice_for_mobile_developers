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

#include "vulkan_sample.h"

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include "common/glm_common.h"
#include <imgui.h>
VKBP_ENABLE_WARNINGS()

#include "common/helpers.h"
#include "common/logging.h"
#include "common/vk_common.h"
#include "gltf_loader.h"
#include "platform/platform.h"
#include "platform/window.h"
#include "scene_graph/components/camera.h"
#include "utils/graphs.h"
#include "utils/strings.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

namespace vkb
{
VulkanSample::VulkanSample()
{
}

VulkanSample::~VulkanSample()
{
	device->wait_idle();

	scene.reset();

	stats.reset();
	gui.reset();
	render_context.reset();
	device.reset();

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance->get_handle(), surface, nullptr);
	}

	instance.reset();
}

bool VulkanSample::prepare(Platform &platform)
{
	if (!Application::prepare(platform))
	{
		return false;
	}

	LOGI("Initializing Vulkan sample");

	// Creating the vulkan instance
	std::vector<const char *> instance_extensions = get_instance_extensions();
	instance_extensions.push_back(platform.get_surface_extension());
	instance = std::make_unique<Instance>(get_name(), instance_extensions, get_validation_layers(), is_headless());

	// Getting a valid vulkan surface from the platform
	surface = platform.get_window().create_surface(instance->get_handle());

	// Creating vulkan device, specifying the swapchain
	std::vector<const char *> device_extensions = get_device_extensions();
	if (!is_headless() || instance->is_enabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
	{
		device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
	device = std::make_unique<vkb::Device>(instance->get_gpu(), surface, device_extensions);

	// Preparing render context for rendering
	render_context = std::make_unique<vkb::RenderContext>(*device, surface, platform.get_window().get_width(), platform.get_window().get_height());
	prepare_render_context();

	return true;
}

void VulkanSample::prepare_render_context()
{
	render_context->prepare();
}

void VulkanSample::update_scene(float delta_time)
{
	if (scene)
	{
		//Update scripts
		if (scene->has_component<sg::Script>())
		{
			auto scripts = scene->get_components<sg::Script>();

			for (auto script : scripts)
			{
				script->update(delta_time);
			}
		}
	}
}

void VulkanSample::update_stats(float delta_time)
{
	if (stats)
	{
		stats->update();

		static float stats_view_count = 0.0f;
		stats_view_count += delta_time;

		// Reset every STATS_VIEW_RESET_TIME seconds
		if (stats_view_count > STATS_VIEW_RESET_TIME)
		{
			reset_stats_view();
			stats_view_count = 0.0f;
		}
	}
}

void VulkanSample::update_gui(float delta_time)
{
	if (gui)
	{
		if (gui->is_debug_view_active())
		{
			update_debug_window();
		}

		gui->new_frame();

		gui->show_top_window(get_name(), stats.get(), &get_debug_info());

		// Samples can override this
		draw_gui();

		gui->update(delta_time);
	}
}

void VulkanSample::update(float delta_time)
{
	update_scene(delta_time);

	update_stats(delta_time);

	update_gui(delta_time);

	auto &command_buffer = render_context->begin();

	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	draw(command_buffer, render_context->get_active_frame().get_render_target());

	command_buffer.end();

	render_context->submit(command_buffer);
}

void VulkanSample::draw(CommandBuffer &command_buffer, RenderTarget &render_target)
{
	auto &views = render_target.get_views();

	{
		// Image 0 is the swapchain
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		memory_barrier.src_access_mask = 0;
		memory_barrier.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		command_buffer.image_memory_barrier(views.at(0), memory_barrier);

		// Skip 1 as it is handled later as a depth-stencil attachment
		for (size_t i = 2; i < views.size(); ++i)
		{
			command_buffer.image_memory_barrier(views.at(i), memory_barrier);
		}
	}

	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		memory_barrier.src_access_mask = 0;
		memory_barrier.dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		command_buffer.image_memory_barrier(views.at(1), memory_barrier);
	}

	draw_renderpass(command_buffer, render_target);

	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		command_buffer.image_memory_barrier(views.at(0), memory_barrier);
	}
}

void VulkanSample::draw_renderpass(CommandBuffer &command_buffer, RenderTarget &render_target)
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

	render(command_buffer);

	if (gui)
	{
		gui->draw(command_buffer);
	}

	command_buffer.end_render_pass();
}

void VulkanSample::render(CommandBuffer &command_buffer)
{
	if (render_pipeline)
	{
		render_pipeline->draw(command_buffer, render_context->get_active_frame().get_render_target());
	}
}

void VulkanSample::resize(uint32_t width, uint32_t height)
{
	Application::resize(width, height);

	if (gui)
	{
		gui->resize(width, height);
	}

	if (scene->has_component<sg::Script>())
	{
		auto scripts = scene->get_components<sg::Script>();

		for (auto script : scripts)
		{
			script->resize(width, height);
		}
	}

	if (stats)
	{
		stats->resize(width);
	}
}

void VulkanSample::input_event(const InputEvent &input_event)
{
	Application::input_event(input_event);

	bool gui_captures_event = false;

	if (gui)
	{
		gui_captures_event = gui->input_event(input_event);
	}

	if (!gui_captures_event)
	{
		if (scene->has_component<sg::Script>())
		{
			auto scripts = scene->get_components<sg::Script>();

			for (auto script : scripts)
			{
				script->input_event(input_event);
			}
		}
	}

	if (input_event.get_source() == EventSource::Keyboard)
	{
		const auto &key_event = static_cast<const KeyInputEvent &>(input_event);
		if (key_event.get_action() == KeyAction::Down && key_event.get_code() == KeyCode::PrintScreen)
		{
			screenshot(*render_context, "screenshot-" + get_name());
		}

		if (key_event.get_code() == KeyCode::F6 && key_event.get_action() == KeyAction::Down)
		{
			utils::debug_graphs(get_render_context(), *scene.get());
		}
	}
}

void VulkanSample::finish()
{
	Application::finish();
	device->wait_idle();
}

Device &VulkanSample::get_device()
{
	return *device;
}

Configuration &VulkanSample::get_configuration()
{
	return configuration;
}

void VulkanSample::draw_gui()
{
}

void VulkanSample::update_debug_window()
{
	auto        driver_version     = device->get_driver_version();
	std::string driver_version_str = fmt::format("major: {} minor: {} patch: {}", driver_version.major, driver_version.minor, driver_version.patch);
	get_debug_info().insert<field::Static, std::string>("driver_version", driver_version_str);

	get_debug_info().insert<field::Static, std::string>("resolution",
	                                                    utils::to_string(render_context->get_swapchain().get_extent()));

	get_debug_info().insert<field::Static, std::string>("surface_format",
	                                                    utils::to_string(render_context->get_swapchain().get_format()) + " (" +
	                                                        to_string(get_bits_per_pixel(render_context->get_swapchain().get_format())) + "bbp)");

	get_debug_info().insert<field::Static, uint32_t>("mesh_count", to_u32(scene->get_components<sg::SubMesh>().size()));

	get_debug_info().insert<field::Static, uint32_t>("texture_count", to_u32(scene->get_components<sg::Texture>().size()));

	if (auto camera = scene->get_components<vkb::sg::Camera>().at(0))
	{
		if (auto camera_node = camera->get_node())
		{
			const glm::vec3 &pos = camera_node->get_transform().get_translation();
			get_debug_info().insert<field::Vector, float>("camera_pos", pos.x, pos.y, pos.z);
		}
	}
}

void VulkanSample::load_scene(const std::string &path)
{
	GLTFLoader loader{*device};

	scene = loader.read_scene_from_file(path);

	if (!scene)
	{
		LOGE("Cannot load scene: {}", path.c_str());
		throw std::runtime_error("Cannot load scene: " + path);
	}
}

VkSurfaceKHR VulkanSample::get_surface()
{
	return surface;
}

RenderContext &VulkanSample::get_render_context()
{
	assert(render_context && "Render context is not valid");
	return *render_context;
}

void VulkanSample::set_render_pipeline(RenderPipeline &&rp)
{
	render_pipeline.reset();
	render_pipeline = std::make_unique<RenderPipeline>(std::move(rp));
}

RenderPipeline &VulkanSample::get_render_pipeline()
{
	assert(render_pipeline && "Render pipeline was not created");
	return *render_pipeline;
}

std::vector<const char *> VulkanSample::get_validation_layers()
{
	return {};
}

std::vector<const char *> VulkanSample::get_instance_extensions()
{
	return {};
}

std::vector<const char *> VulkanSample::get_device_extensions()
{
	return {};
}

sg::Scene &VulkanSample::get_scene()
{
	assert(scene && "Scene not loaded");
	return *scene;
}
}        // namespace vkb
