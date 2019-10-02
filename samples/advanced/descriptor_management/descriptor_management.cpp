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

#include "descriptor_management.h"

#include "common/vk_common.h"
#include "gltf_loader.h"
#include "gui.h"
#include "platform/filesystem.h"
#include "platform/platform.h"
#include "rendering/subpasses/forward_subpass.h"
#include "stats.h"

DescriptorManagement::DescriptorManagement()
{
	auto &config = get_configuration();

	config.insert<vkb::IntSetting>(0, descriptor_caching.value, 0);
	config.insert<vkb::IntSetting>(0, buffer_allocation.value, 0);

	config.insert<vkb::IntSetting>(1, descriptor_caching.value, 1);
	config.insert<vkb::IntSetting>(1, buffer_allocation.value, 1);
}

bool DescriptorManagement::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	// Load a scene from the assets folder
	load_scene("scenes/bonza/Bonza4X.gltf");

	// Attach a move script to the camera component in the scene
	auto &camera_node = add_free_camera("main_camera");
	camera            = dynamic_cast<vkb::sg::PerspectiveCamera *>(&camera_node.get_component<vkb::sg::Camera>());

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

void DescriptorManagement::update(float delta_time)
{
	update_scene(delta_time);

	update_stats(delta_time);

	update_gui(delta_time);

	auto &render_context = get_render_context();

	auto &command_buffer = render_context.begin();

	// Process GUI input
	auto buffer_alloc_strategy = (buffer_allocation.value == 0) ?
	                                 vkb::BufferAllocationStrategy::OneAllocationPerBuffer :
	                                 vkb::BufferAllocationStrategy::MultipleAllocationsPerBuffer;

	render_context.get_active_frame().set_buffer_allocation_strategy(buffer_alloc_strategy);

	if (descriptor_caching.value == 0)
	{
		// Clear descriptor pools for the current frame
		render_context.get_active_frame().clear_descriptors();
	}

	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	draw(command_buffer, render_context.get_active_frame().get_render_target());

	command_buffer.end();

	render_context.submit(command_buffer);
}

void DescriptorManagement::draw_gui()
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

std::unique_ptr<vkb::VulkanSample> create_descriptor_management()
{
	return std::make_unique<DescriptorManagement>();
}
