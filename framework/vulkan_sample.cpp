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

#include "gltf_loader.h"
#include "platform/platform.h"

#include <imgui.h>

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "gltf_loader.h"
#	include "platform/android/android_platform.h"
#endif

#include "scene_graph/script.h"
#include "scene_graph/scripts/free_camera.h"

namespace vkb
{
namespace
{
#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type,
                                                     uint64_t object, size_t location, int32_t message_code,
                                                     const char *layer_prefix, const char *message, void *user_data)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		LOGE("Validation Layer: Error: {}: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		LOGE("Validation Layer: Warning: {}: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		LOGI("Validation Layer: Performance warning: {}: {}", layer_prefix, message);
	}
	else
	{
		LOGI("Validation Layer: Information: {}: {}", layer_prefix, message);
	}
	return VK_FALSE;
}
#endif
bool validate_extensions(const std::vector<const char *> &         required,
                         const std::vector<VkExtensionProperties> &available)
{
	for (auto extension : required)
	{
		bool found = false;
		for (auto &available_extension : available)
		{
			if (strcmp(available_extension.extensionName, extension) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}

bool validate_layers(const std::vector<const char *> &     required,
                     const std::vector<VkLayerProperties> &available)
{
	for (auto extension : required)
	{
		bool found = false;
		for (auto &available_extension : available)
		{
			if (strcmp(available_extension.layerName, extension) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}
}        // namespace

VulkanSample::VulkanSample()
{
}

VulkanSample::~VulkanSample()
{
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
}

bool VulkanSample::prepare(Platform &platform)
{
	if (!Application::prepare(platform))
	{
		return false;
	}

	get_debug_info().insert<field::MinMax, float>("fps", fps);
	get_debug_info().insert<field::MinMax, float>("frame_time", frame_time);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	auto &android_platform  = dynamic_cast<AndroidPlatform &>(platform);
	tinygltf::asset_manager = android_platform.get_activity()->assetManager;
#endif

	LOGI("Initializing context");

	instance = create_instance({VK_KHR_SURFACE_EXTENSION_NAME});
	surface  = platform.create_surface(instance);

	uint32_t physical_device_count{0};
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));

	if (physical_device_count < 1)
	{
		LOGE("Failed to enumerate Vulkan physical device.");
		return false;
	}

	gpus.resize(physical_device_count);

	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, gpus.data()));

	return true;
}

void VulkanSample::update(float delta_time)
{
	if (scene.has_component<sg::Script>())
	{
		auto scripts = scene.get_components<sg::Script>();

		for (auto script : scripts)
		{
			script->update(delta_time);
		}
	}

	get_debug_info().insert<field::Static, std::string>("resolution",
	                                                    to_string(render_context->get_swapchain().get_extent().width) + "x" +
	                                                        to_string(render_context->get_swapchain().get_extent().height));

	get_debug_info().insert<field::Static, std::string>("surface_format",
	                                                    convert_format_to_string(render_context->get_swapchain().get_format()) + " (" +
	                                                        to_string(vkb::get_bits_per_pixel(render_context->get_swapchain().get_format())) + "bbp)");

	get_debug_info().insert<field::Static, uint32_t>("mesh_count", to_u32(scene.get_components<sg::SubMesh>().size()));

	get_debug_info().insert<field::Static, uint32_t>("texture_count", to_u32(scene.get_components<sg::Texture>().size()));

	VkSemaphore aquired_semaphore = render_context->begin_frame();

	if (aquired_semaphore == VK_NULL_HANDLE)
	{
		return;
	}

	const auto &queue = device->get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);

	CommandBuffer &cmd_buf = render_context->request_frame_command_buffer(queue);

	// Update stats
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

	// Update gui
	if (gui)
	{
		gui->new_frame();

		gui->show_top_window(get_name(), stats.get(), &debug_info);

		// Samples can override this
		draw_gui();

		gui->update(delta_time);
	}

	const RenderTarget &render_target = render_context->get_active_frame().get_render_target();

	cmd_buf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memory_barrier.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		cmd_buf.image_memory_barrier(render_target.get_views().at(0), memory_barrier);
	}

	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		memory_barrier.src_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		memory_barrier.dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		cmd_buf.image_memory_barrier(render_target.get_views().at(1), memory_barrier);
	}

	draw_swapchain_renderpass(cmd_buf, render_target);

	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		cmd_buf.image_memory_barrier(render_target.get_views().at(0), memory_barrier);
	}

	cmd_buf.end();

	VkSemaphore render_semaphore = render_context->submit(queue, cmd_buf, aquired_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	render_context->end_frame(render_semaphore);
}

void VulkanSample::resize(uint32_t width, uint32_t height)
{
	Application::resize(width, height);

	if (gui)
	{
		gui->resize(width, height);
	}

	if (scene.has_component<sg::Script>())
	{
		auto scripts = scene.get_components<sg::Script>();

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
		if (scene.has_component<sg::Script>())
		{
			auto scripts = scene.get_components<sg::Script>();

			for (auto script : scripts)
			{
				script->input_event(input_event);
			}
		}
	}
}

VkPhysicalDevice VulkanSample::get_gpu(size_t i)
{
	return gpus.at(i);
}

VkSurfaceKHR VulkanSample::get_surface()
{
	return surface;
}

void VulkanSample::draw_scene(vkb::CommandBuffer &command_buffer)
{
}

void VulkanSample::draw_gui()
{
}

sg::Node &VulkanSample::add_free_camera(const std::string &node_name)
{
	auto camera_node = scene.find_node(node_name);

	if (!camera_node)
	{
		LOGW("Camera node `{}` not found. Looking for `default_camera` node.", node_name.c_str());

		camera_node = scene.find_node("default_camera");
	}

	if (!camera_node)
	{
		throw std::runtime_error("Camera node with name `" + node_name + "` not found.");
	}

	if (!camera_node->has_component<sg::Camera>())
	{
		throw std::runtime_error("No camera component found for `" + node_name + "` node.");
	}

	auto free_camera_script = std::make_unique<vkb::sg::FreeCamera>(*camera_node);

	scene.add_component(std::move(free_camera_script), *camera_node);

	return *camera_node;
}

void VulkanSample::load_scene(const std::string &path)
{
	vkb::GLTFLoader loader{*device};

	bool status = loader.read_scene_from_file(path, scene);

	if (!status)
	{
		LOGE("Cannot load scene: {}", path.c_str());
		throw std::runtime_error("Cannot load scene: " + path);
	}
}

VkInstance VulkanSample::create_instance(const std::vector<const char *> &required_instance_extensions,
                                         const std::vector<const char *> &required_instance_layers)
{
	VkResult result = volkInitialize();
	if (result)
	{
		throw VulkanException(result, "Failed to initialize volk.");
	}

	uint32_t instance_extension_count;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));

	std::vector<VkExtensionProperties> instance_extensions(instance_extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

	std::vector<const char *> active_instance_extensions(required_instance_extensions);

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	active_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	active_instance_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	active_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	active_instance_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	active_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	if (!validate_extensions(active_instance_extensions, instance_extensions))
	{
		throw std::runtime_error("Required instance extensions are missing.");
	}

	uint32_t instance_layer_count;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));

	std::vector<VkLayerProperties> instance_layers(instance_layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

	std::vector<const char *> active_instance_layers(required_instance_layers);

#ifdef VKB_VALIDATION_LAYERS
	active_instance_layers.push_back("VK_LAYER_GOOGLE_threading");
	active_instance_layers.push_back("VK_LAYER_LUNARG_parameter_validation");
	active_instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
	active_instance_layers.push_back("VK_LAYER_LUNARG_core_validation");
	active_instance_layers.push_back("VK_LAYER_GOOGLE_unique_objects");
#endif

	if (!validate_layers(active_instance_layers, instance_layers))
	{
		throw std::runtime_error("Required instance layers are missing.");
	}

	VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};

	app_info.pApplicationName   = get_name().c_str();
	app_info.applicationVersion = 0;
	app_info.pEngineName        = "Vulkan Best Practice";
	app_info.engineVersion      = 0;
	app_info.apiVersion         = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instance_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

	instance_info.pApplicationInfo = &app_info;

	instance_info.enabledExtensionCount   = to_u32(active_instance_extensions.size());
	instance_info.ppEnabledExtensionNames = active_instance_extensions.data();

	instance_info.enabledLayerCount   = to_u32(active_instance_layers.size());
	instance_info.ppEnabledLayerNames = active_instance_layers.data();

	// Create the Vulkan instance

	VkInstance instance;
	result = vkCreateInstance(&instance_info, nullptr, &instance);
	if (result != VK_SUCCESS)
	{
		throw VulkanException(result, "Could not create Vulkan instance");
	}

	volkLoadInstance(instance);

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	VkDebugReportCallbackCreateInfoEXT info = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};

	info.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	info.pfnCallback = debug_callback;

	result = vkCreateDebugReportCallbackEXT(instance, &info, nullptr, &debug_report_callback);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Could not create debug callback.");
	}
#endif

	return instance;
}

void VulkanSample::draw_swapchain_renderpass(vkb::CommandBuffer &command_buffer, const RenderTarget &render_target)
{
	std::vector<vkb::LoadStoreInfo> load_store{2};
	load_store[0].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[0].store_op = VK_ATTACHMENT_STORE_OP_STORE;
	load_store[1].load_op  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	load_store[1].store_op = VK_ATTACHMENT_STORE_OP_STORE;

	std::vector<VkClearValue> clear_value{2};
	clear_value[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
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

	// Draw gui
	if (gui)
	{
		gui->draw(command_buffer);
	}

	command_buffer.end_render_pass();
}
}        // namespace vkb
