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

#include "profiler.h"

#include <cstdint>
#include <ctime>
#include <set>
#include <vector>

namespace vkb
{
/**
 * @brief Handles of stats to be optionally enabled in @ref Stats
 */
enum class StatIndex
{
	frame_times,
	cpu_cycles,
	cpu_instructions,
	cache_miss_ratio,
	branch_miss_ratio,
	gpu_cycles,
	vertex_compute_cycles,
	fragment_cycles,
	l2_reads_lookups,
	l2_ext_reads,
	l2_writes_lookups,
	l2_ext_writes,
	l2_ext_read_stalls,
	l2_ext_write_stalls,
	l2_ext_read_beats,
	l2_ext_write_beats
};

/*
 * @brief Helper class for querying statistics about the CPU and the GPU
 */
class Stats
{
  public:
	/**
	 * @brief Constructs a Stats object
	 * @param profiler
	 * @param enabled_stats Set of stats to be collected
	 * @param buffer_size Size of the circular buffers
	 */
	Stats(Profiler &profiler, const std::set<StatIndex> &enabled_stats, const size_t buffer_size = 16);

	/**
	 * @brief Resizes the stats buffers according to the width of the screen
	 * @param width The width of the screen
	 */
	void resize(const size_t width);

	/**
	 * @brief Checks if an enabled stat is available in the current platform
	 * @param index The stat index
	 * @return True if the stat is enabled and available, false otherwise
	 */
	bool is_available(const StatIndex index) const;

	/**
	 * @param index The stat index of the data requested
	 * @return The data of the specified stat
	 */
	const std::vector<float> &get_data(const StatIndex index) const;

	/**
	 * @return The enabled stats
	 */
	const std::set<StatIndex> &get_enabled_stats() const
	{
		return enabled_stats;
	}

	/**
	 * @return The frame times circular buffer
	 */
	const std::vector<float> &get_frame_times() const
	{
		return frame_times;
	}

	/**
	 * @return The number of CPU cycles in the frame
	 */
	const std::vector<float> &get_cpu_cycles() const
	{
		return cpu_cycles;
	}

	/**
	 * @return The number of CPU instructions in the frame
	 */
	const std::vector<float> &get_cpu_instructions() const
	{
		return cpu_instructions;
	}

	/**
	 * @return The cache miss ratio for the frame
	 */
	const std::vector<float> &get_cache_miss_ratio() const
	{
		return cache_miss_ratio;
	}

	/**
	 * @return The branch miss ratio for the frame
	 */
	const std::vector<float> &get_branch_miss_ratio() const
	{
		return branch_miss_ratio;
	}

	/**
	 * @return The number of GPU cycles in the frame
	 */
	const std::vector<float> &get_gpu_cycles() const
	{
		return gpu_cycles;
	}

	/**
	 * @return The number of vertex/compute cycles in the frame
	 */
	const std::vector<float> &get_vertex_compute_cycles() const
	{
		return vertex_compute_cycles;
	}

	/**
	 * @return The number of fragment cycles in the frame
	 */
	const std::vector<float> &get_fragment_cycles() const
	{
		return fragment_cycles;
	}

	/**
	 * @return The number of L2 read lookups in the frame
	 */
	const std::vector<float> &get_l2_read_lookups() const
	{
		return l2_reads_lookups;
	}

	/**
	 * @return The number of L2 external reads in the frame
	 */
	const std::vector<float> &get_l2_ext_reads() const
	{
		return l2_ext_reads;
	}

	/**
	 * @return The number of L2 write lookups in the frame
	 */
	const std::vector<float> &get_l2_write_lookups() const
	{
		return l2_writes_lookups;
	}

	/**
	 * @return The number of L2 external writes in the frame
	 */
	const std::vector<float> &get_l2_ext_writes() const
	{
		return l2_ext_writes;
	}

	/**
	 * @return The number of L2 external read stalls in the frame
	 */
	const std::vector<float> &get_l2_ext_read_stalls() const
	{
		return l2_ext_read_stalls;
	}

	/**
	 * @return The number of L2 external write stalls in the frame
	 */
	const std::vector<float> &get_l2_ext_write_stalls() const
	{
		return l2_ext_write_stalls;
	}

	/**
	 * @return The number of L2 external read beats in the frame
	 */
	const std::vector<float> &get_l2_ext_read_beats() const
	{
		return l2_ext_read_beats;
	}

	/**
	 * @return The number of L2 external write beats in the frame
	 */
	const std::vector<float> &get_l2_ext_write_beats() const
	{
		return l2_ext_write_beats;
	}

	/**
	 * @brief Update statistics, must be called after every frame
	 */
	void update();

  private:
	/// Stats to be enabled
	std::set<StatIndex> enabled_stats;

	/// Available stats for the current platform (only checked for enabled stats)
	std::set<StatIndex> available_stats;

	/// Time of previous frame
	std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;

	/// Frame counter incremented by one every Update call
	std::vector<float> frame_times = {};

	/// Number of CPU cycles in the frame
	std::vector<float> cpu_cycles = {};

	/// Number of CPU instructions in the frame
	std::vector<float> cpu_instructions = {};

	/// Cache miss ratio for the frame
	std::vector<float> cache_miss_ratio = {};

	/// Branch miss ratio for the frame
	std::vector<float> branch_miss_ratio = {};

	/// Number of GPU cycles in the frame
	std::vector<float> gpu_cycles = {};

	/// Number of vertex/compute cycles in the frame
	std::vector<float> vertex_compute_cycles = {};

	/// Number of fragment cycles in the frame
	std::vector<float> fragment_cycles = {};

	/// Number of L2 read lookups in the frame
	std::vector<float> l2_reads_lookups = {};

	/// Number of L2 external reads in the frame
	std::vector<float> l2_ext_reads = {};

	/// Number of L2 write lookups in the frame
	std::vector<float> l2_writes_lookups = {};

	/// Number of L2 external writes in the frame
	std::vector<float> l2_ext_writes = {};

	/// Number of L2 external read stalls in the frame
	std::vector<float> l2_ext_read_stalls = {};

	/// Number of L2 external write stalls in the frame
	std::vector<float> l2_ext_write_stalls = {};

	/// Number of L2 external read beats in the frame
	std::vector<float> l2_ext_read_beats = {};

	/// Number of L2 external write beats in the frame
	std::vector<float> l2_ext_write_beats = {};

	float cpu_cycles_sum            = 0;
	float cpu_instructions_sum      = 0;
	float cache_miss_ratio_average  = 0;
	float branch_miss_ratio_average = 0;
	float gpu_cycles_sum            = 0;
	float vertex_compute_cycles_sum = 0;
	float fragment_cycles_sum       = 0;
	float l2_reads_lookups_sum      = 0;
	float l2_ext_reads_sum          = 0;
	float l2_writes_lookups_sum     = 0;
	float l2_ext_writes_sum         = 0;
	float l2_ext_read_stalls_sum    = 0;
	float l2_ext_write_stalls_sum   = 0;
	float l2_ext_read_beats_sum     = 0;
	float l2_ext_write_beats_sum    = 0;

	/// Profiler to gather CPU and GPU performance data
	Profiler &profiler;
};

}        // namespace vkb
