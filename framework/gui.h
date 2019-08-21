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
#include <future>
#include <imgui.h>
#include <imgui_internal.h>
#include <thread>

#include "core/command_buffer.h"
#include "core/sampler.h"
#include "debug_info.h"
#include "platform/filesystem.h"
#include "platform/input_events.h"
#include "stats.h"

namespace vkb
{
/**
 * @brief Helper structure for fonts loaded from TTF
 */
struct Font
{
	/**
	 * @brief Constructor
	 * @param name The name of the font file that exists within 'assets/fonts' (without extension)
	 * @param size The font size, scaled by DPI
	 */
	Font(const std::string &name, float size) :
	    name{name},
	    data{vkb::fs::read_asset("fonts/" + name + ".ttf")},
	    size{size}
	{
		// Keep ownership of the font data to avoid a double delete
		ImFontConfig font_config{};
		font_config.FontDataOwnedByAtlas = false;

		ImGuiIO &io = ImGui::GetIO();
		handle      = io.Fonts->AddFontFromMemoryTTF(data.data(), static_cast<int>(data.size()), size, &font_config);
	}

	ImFont *handle{nullptr};

	std::string name;

	std::vector<uint8_t> data;

	float size{};
};

class VulkanSample;

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
		     {/* label = */ "Frame time: {:3.1f} ms",
		      /* scale_factor = */ 1000.0f}},
		    {StatIndex::cpu_cycles,
		     {/* label = */ "CPU cycles: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::cpu_instructions,
		     {/* label = */ "CPU inst: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::cache_miss_ratio,
		     {/* label = */ "Cache misses: {:3.1f}%",
		      /* scale_factor = */ 100.0f,
		      /* has_fixed_max = */ true,
		      /* max_value = */ 100.0f}},
		    {StatIndex::branch_miss_ratio,
		     {/* label = */ "Branch misses: {:3.1f}%",
		      /* scale_factor = */ 100.0f,
		      /* has_fixed_max = */ true,
		      /* max_value = */ 100.0f}},

		    {StatIndex::gpu_cycles,
		     {/* label = */ "GPU cycles: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::vertex_compute_cycles,
		     {/* label = */ "Vert cycles: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::tiles,
		     {/* label = */ "Tiles: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::fragment_cycles,
		     {/* label = */ "Frag cycles: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::fragment_jobs,
		     {/* label = */ "Frag jobs: {:4.0f}/s"}},
		    {StatIndex::tex_instr,
		     {/* label = */ "Tex instr: {:4.0f} k/s",
		      /* scale_factor = */ float(1e-3)}},
		    {StatIndex::l2_ext_reads,
		     {/* label = */ "Ext reads: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_writes,
		     {/* label = */ "Ext writes: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_read_stalls,
		     {/* label = */ "Ext read stalls: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_write_stalls,
		     {/* label = */ "Ext write stalls: {:4.1f} M/s",
		      /* scale_factor = */ float(1e-6)}},
		    {StatIndex::l2_ext_read_bytes,
		     {/* label = */ "Ext read bw: {:4.1f} MiB/s",
		      /* scale_factor = */ 1.0f / (1024.0f * 1024.0f)}},
		    {StatIndex::l2_ext_write_bytes,
		     {/* label = */ "Ext write bw: {:4.1f} MiB/s",
		      /* scale_factor = */ 1.0f / (1024.0f * 1024.0f)}}};

		float graph_height{50.0f};

		float top_padding{1.1f};
	};

	/**
	 * @brief Helper class for rendering debug statistics in the GUI
	 */
	class DebugView
	{
	  public:
		bool active{false};

		float scale{1.7f};

		uint32_t max_fields{8};

		float label_column_width{0};
	};

	// The name of the default font file to use
	static const std::string default_font;

	/**
	 * @brief Initializes the Gui
	 * @param sample_ A vulkan render context
	 * @param dpi_factor The dpi scale factor
	 */
	Gui(VulkanSample &sample_, const float dpi_factor = 1.0);

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
	 * @param delta_time Time passed since last update
	 */
	void update(const float delta_time);

	/**
	 * @brief Draws the Gui
	 * @param command_buffer Command buffer to register draw-commands
	 */
	void draw(CommandBuffer &command_buffer);

	/**
	 * @brief Shows an overlay top window with app info and maybe stats
	 * @param app_name Application name
	 * @param stats Statistics to show (can be null)
	 * @param debug_info Debug info to show (can be null)
	 */
	void show_top_window(const std::string &app_name, const Stats *stats = nullptr, DebugInfo *debug_info = nullptr);

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
	 * @brief Shows a moveable window with debug information
	 * @param debug_info The object holding the data fields to be displayed
	 * @param position The absolute position to set
	 */
	void show_debug_window(DebugInfo &debug_info, const ImVec2 &position);

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

	Font &get_font(const std::string &font_name = Gui::default_font);

	bool is_debug_view_active() const;

  private:
	/**
	 * @brief Updates Vulkan buffers
	 * @param frame Frame to render into
	 */
	void update_buffers(CommandBuffer &command_buffer);

	static const double press_time_ms;

	static const float overlay_alpha;

	static const ImGuiWindowFlags common_flags;

	static const ImGuiWindowFlags options_flags;

	static const ImGuiWindowFlags info_flags;

	VulkanSample &sample;

	/// Scale factor to apply to the size of gui elements (expressed in dp)
	float dpi_factor{1.0f};

	std::vector<Font> fonts;

	std::unique_ptr<core::Image>     font_image;
	std::unique_ptr<core::ImageView> font_image_view;

	std::unique_ptr<core::Sampler> sampler{nullptr};

	PipelineLayout *pipeline_layout{nullptr};

	StatsView stats_view;

	DebugView debug_view;

	/// Used to measure duration of input events
	Timer timer;

	/// Used to show/hide the GUI
	bool visible = true;

	/// Whether or not the GUI has detected a multi touch gesture
	bool two_finger_tap = false;

	bool show_graph_file_output = false;
};

void Gui::new_frame()
{
	ImGui::NewFrame();
}
}        // namespace vkb
