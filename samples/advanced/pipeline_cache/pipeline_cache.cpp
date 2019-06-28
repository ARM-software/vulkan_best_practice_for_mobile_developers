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

#include "pipeline_cache.h"

#include <imgui_internal.h>

#include "common/logging.h"
#include "core/device.h"
#include "gui.h"
#include "platform/file.h"
#include "platform/platform.h"
#include "rendering/subpasses/scene_subpass.h"
#include "scene_graph/node.h"
#include "stats.h"

PipelineCache::~PipelineCache()
{
	if (pipeline_cache != VK_NULL_HANDLE)
	{
		/* Get size of pipeline cache */
		size_t size{};
		VK_CHECK(vkGetPipelineCacheData(device->get_handle(), pipeline_cache, &size, nullptr));

		/* Get data of pipeline cache */
		std::vector<uint8_t> data(size);
		VK_CHECK(vkGetPipelineCacheData(device->get_handle(), pipeline_cache, &size, data.data()));

		/* Write pipeline cache data to a file in binary format */
		vkb::file::write_temp(data, "pipeline_cache.data");

		/* Destroy Vulkan pipeline cache */
		vkDestroyPipelineCache(device->get_handle(), pipeline_cache, nullptr);
	}

	vkb::file::write_temp(device->get_resource_cache().serialize(), "cache.data");
}

bool PipelineCache::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	/* Try to read pipeline cache file if exists */
	std::vector<uint8_t> pipeline_data;

	try
	{
		pipeline_data = vkb::file::read_temp("pipeline_cache.data");
	}
	catch (std::runtime_error &ex)
	{
		LOGW("No pipeline cache found. {}", ex.what());
	}

	std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	device = std::make_unique<vkb::Device>(get_gpu(), get_surface(), extensions);

	/* Add initial pipeline cache data from the cached file */
	VkPipelineCacheCreateInfo create_info{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
	create_info.initialDataSize = pipeline_data.size();
	create_info.pInitialData    = pipeline_data.data();

	/* Create Vulkan pipeline cache */
	VK_CHECK(vkCreatePipelineCache(device->get_handle(), &create_info, nullptr, &pipeline_cache));

	vkb::ResourceCache &resource_cache = device->get_resource_cache();

	// Use pipeline cache to store pipelines
	resource_cache.set_pipeline_cache(pipeline_cache);

	std::vector<uint8_t> data_cache;

	try
	{
		data_cache = vkb::file::read_temp("cache.data");
	}
	catch (std::runtime_error &ex)
	{
		LOGW("No data cache found. {}", ex.what());
	}

	// Build all pipelines from a previous run
	resource_cache.warmup(data_cache);

	auto swapchain = std::make_unique<vkb::Swapchain>(*device, get_surface());

	stats = std::make_unique<vkb::Stats>(std::set<vkb::StatIndex>{vkb::StatIndex::frame_times});

	render_context = std::make_unique<vkb::RenderContext>(std::move(swapchain));

	float dpi_factor = platform.get_dpi_factor();

	button_size.x = button_size.x * dpi_factor;
	button_size.y = button_size.y * dpi_factor;

	gui = std::make_unique<vkb::Gui>(*render_context, dpi_factor);

	load_scene("scenes/sponza/Sponza01.gltf");
	auto &camera_node = add_free_camera("main_camera");
	camera            = &camera_node.get_component<vkb::sg::Camera>();

	vkb::ShaderSource vert_shader(vkb::file::read_asset("shaders/base.vert"));
	vkb::ShaderSource frag_shader(vkb::file::read_asset("shaders/base.frag"));
	auto              scene_subpass = std::make_unique<vkb::SceneSubpass>(*render_context, std::move(vert_shader), std::move(frag_shader), scene, *camera);

	render_pipeline = std::make_unique<vkb::RenderPipeline>();
	render_pipeline->add_subpass(std::move(scene_subpass));

	return true;
}

void PipelineCache::draw_gui()
{
	gui->show_options_window(
	    /* body = */ [this]() {
		    if (ImGui::Checkbox("Pipeline cache", &enable_pipeline_cache))
		    {
			    vkb::ResourceCache &resource_cache = device->get_resource_cache();

			    if (enable_pipeline_cache)
			    {
				    // Use pipeline cache to store pipelines
				    resource_cache.set_pipeline_cache(pipeline_cache);
			    }
			    else
			    {
				    // Don't use a pipeline cache
				    resource_cache.set_pipeline_cache(VK_NULL_HANDLE);
			    }
		    }

		    ImGui::SameLine();

		    if (ImGui::Button("Destroy Pipelines", button_size))
		    {
			    device->wait_idle();
			    device->get_resource_cache().clear_pipelines();
			    record_frame_time_next_frame = true;
		    }

		    if (rebuild_pipelines_frame_time_ms > 0.0f)
		    {
			    ImGui::Text("Pipeline rebuild frame time: %.1f ms", rebuild_pipelines_frame_time_ms);
		    }
		    else
		    {
			    ImGui::Text("Pipeline rebuild frame time: N/A");
		    }
	    },
	    /* lines = */ 2);
}

void PipelineCache::update(float delta_time)
{
	if (record_frame_time_next_frame)
	{
		rebuild_pipelines_frame_time_ms = delta_time * 1000.0f;
		record_frame_time_next_frame    = false;
	}

	VulkanSample::update(delta_time);
}

void PipelineCache::draw_scene(vkb::CommandBuffer &cmd_buf)
{
	render_pipeline->draw(cmd_buf);
}

std::unique_ptr<vkb::VulkanSample> create_pipeline_cache()
{
	return std::make_unique<PipelineCache>();
}
