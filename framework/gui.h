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

#pragma once

#include <cstdint>
#include <functional>
#include <imgui.h>

#include "core/command_buffer.h"
#include "platform/input_events.h"
#include "render_context.h"
#include "stats.h"

namespace vkb
{
/**
 * @brief Helper structure for fonts embedded in the source
 */
struct Font
{
	const uint32_t  size;
	const uint32_t *data;
};

/**
 * @brief Vulkan helper class for Dear ImGui
 */
class Gui
{
  public:
	/**
	 * @brief Helper class for drawing statistics
	 */
	class StatsView
	{
	  public:
		/**
		 * @brief Per-statistic graph data
		 */
		class GraphData
		{
		  public:
			/**
			 * @brief Constructs data for the graph
			 * @param graph_label_format Format of the label
			 * @param graph_elements Reference to the data to show
			 * @param scale_factor Any scaling to apply to the data
			 * @param has_fixed_max Whether the data should have a fixed max value
			 * @param max_value The maximum value to use
			 */
			GraphData(const std::string &graph_label_format,
			          float              scale_factor  = 1.0f,
			          bool               has_fixed_max = false,
			          float              max_value     = 0.0f);

			std::string graph_label_format;
			float       scale_factor;
			bool        has_fixed_max;
			float       max_value;
		};

		/**
		 * @brief Resets the max values for the stats
		 *        which do not have a fixed max
		 */
		void reset_max_values();

		/**
		 * @brief Resets the max value for a specific stat
		 */
		void reset_max_value(const StatIndex index);

		/// Per-statistic max values
		std::map<StatIndex, GraphData> graph_map{
		    {StatIndex::frame_times,
		     {/* label = */ "Frame time: %3.1f",
		      /* scale_factor = */ 1}},
		    {StatIndex::cpu_cycles,
		     {/* label = */ "CPU cycles: %4.1f M/s",
		      /* scale_factor = */ float(1e-6),
		      /* has_fixed_max = */ true,
		      /* max_value = */ 10000.0f}},
		    {StatIndex::cpu_instructions,
		     {/* label = */ "CPU inst: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::cache_miss_ratio,
		     {/* label = */ "Cache misses: %3.1f%%",
		      /* scale_factor = */ 100.0f,
		      /* has_fixed_max = */ true,
		      /* max_value = */ 100.0f}},
		    {StatIndex::branch_miss_ratio,
		     {/* label = */ "Branch misses: %3.1f%%",
		      /* scale_factor = */ 100.0f,
		      /* has_fixed_max = */ true,
		      /* max_value = */ 100.0f}},

		    {StatIndex::gpu_cycles,
		     {/* label = */ "GPU cycles: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::vertex_compute_cycles,
		     {/* label = */ "Vert cycles: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::fragment_cycles,
		     {/* label = */ "Frag cycles: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_reads,
		     {/* label = */ "Ext reads: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_writes,
		     {/* label = */ "Ext writes: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_read_stalls,
		     {/* label = */ "Ext read stalls: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_write_stalls,
		     {/* label = */ "Ext write stalls: %4.1f M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_read_beats,
		     {/* label = */ "Ext read bw: %4.1f MiB/s",
		      /* scale_factor = */ 16.0f / (1024.0f * 1024.0f)}},
		    {StatIndex::l2_ext_write_beats,
		     {/* label = */ "Ext write bw: %4.1f MiB/s",
		      /* scale_factor = */ 16.0f / (1024.0f * 1024.0f)}}};
	};

	/**
	 * @brief Initializes the Gui
	 * @param render_context A vulkan render context
	 * @param dpi_factor The dpi scale factor
	 */
	Gui(RenderContext &render_context, const float dpi_factor = 1.0);

	/**
	 * @brief Destroys the Gui
	 */
	~Gui();

	/**
	 * @brief Handles resizing of the window
	 * @param width New width of the window
	 * @param height New height of the window
	 */
	void resize(const uint32_t width, const uint32_t height) const;

	/**
	 * @brief Starts a new ImGui frame
	 *        to be called before drawing any window
	 */
	inline void new_frame();

	/**
	 * @brief Updates the Gui
	 * @param swapchain_index The swapchain index for the image being rendered
	 * @param delta_time Time passed since last update
	 */
	void update(const float delta_time);

	/**
	 * @brief Draws the Gui
	 * @param command_buffer Command buffer to register draw-commands
	 * @param frame Frame to render into
	 */
	void draw(CommandBuffer &command_buffer);

	/**
	 * @brief Shows an overlay top window with app info and maybe stats
	 * @param app_name Application name
	 * @param stats Statistics to show (can be null)
	 */
	void show_top_window(const std::string &app_name, const Stats *stats = nullptr);

	/**
	 * @brief Shows the ImGui Demo window
	 */
	void show_demo_window();

	/**
	 * @brief Shows an child with app info
	 * @param app_name Application name
	 */
	void show_app_info(const std::string &app_name);

	/**
	 * @brief Shows a child with statistics
	 * @param stats Statistics to show
	 */
	void show_stats(const Stats &stats);

	/**
	 * @brief Shows an options windows, to be filled by the sample,
	 *        which will be positioned at the top
	 * @param body ImGui commands defining the body of the window
	 * @param lines The number of lines of text to draw in the window
	 *        These will help the gui to calculate the height of the window
	 */
	void show_options_window(std::function<void()> body, const uint32_t lines = 3);

	bool input_event(const InputEvent &input_event);

	/**
	 * @return The stats view
	 */
	StatsView &get_stats_view();

  private:
	/**
	 * @brief Updates Vulkan buffers
	 * @param frame Frame to render into
	 */
	void update_buffers();

	static const float press_time_ms;

	static const float font_size_dp;

	static const Font roboto_font;

	static const float overlay_alpha;

	static const ImGuiWindowFlags common_flags;

	static const ImGuiWindowFlags options_flags;

	static const ImGuiWindowFlags info_flags;

	RenderContext &render_context;

	/// Scale factor to apply to the size of gui elements (expressed in dp)
	float dpi_factor{1.0f};

	std::unique_ptr<core::Image> font_image;
	std::unique_ptr<ImageView>   font_image_view;

	VkSampler sampler{VK_NULL_HANDLE};

	PipelineLayout &pipeline_layout;

	StatsView stats_view;

	/// Used to measure duration of input events
	std::chrono::time_point<std::chrono::high_resolution_clock> press_start;

	/// Used to show/hide the GUI
	bool visible = true;
};

void Gui::new_frame()
{
	ImGui::NewFrame();
}

}        // namespace vkb
