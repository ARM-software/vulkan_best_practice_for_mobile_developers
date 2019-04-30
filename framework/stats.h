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

#include <hwcpipe.h>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <map>
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
	l2_ext_read_bytes,
	l2_ext_write_bytes
};

struct StatIndexHash
{
	template <typename T>
	std::size_t operator()(T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

enum class StatType
{
	Cpu,
	Gpu,
	Other
};

enum class StatScaling
{
	None,
	ByDeltaTime
};

struct StatData
{
	StatType            type;
	StatScaling         scaling;
	hwcpipe::CpuCounter cpu_counter;
	hwcpipe::GpuCounter gpu_counter;

	StatData(StatScaling stat_scaling = StatScaling::ByDeltaTime) :
	    type(StatType::Other),
	    scaling(stat_scaling)
	{}
	StatData(hwcpipe::CpuCounter c, StatScaling stat_scaling = StatScaling::ByDeltaTime) :
	    type(StatType::Cpu),
	    scaling(stat_scaling),
	    cpu_counter(c)
	{}
	StatData(hwcpipe::GpuCounter c, StatScaling stat_scaling = StatScaling::ByDeltaTime) :
	    type(StatType::Gpu),
	    scaling(stat_scaling),
	    gpu_counter(c)
	{}
};

using StatDataMap = std::unordered_map<StatIndex, StatData, StatIndexHash>;

/*
 * @brief Helper class for querying statistics about the CPU and the GPU
 */
class Stats
{
  public:
	/**
	 * @brief Constructs a Stats object
	 * @param enabled_stats Set of stats to be collected
	 * @param buffer_size Size of the circular buffers
	 */
	Stats(const std::set<StatIndex> &enabled_stats, size_t buffer_size = 16);

	/**
	 * @brief Resizes the stats buffers according to the width of the screen
	 * @param width The width of the screen
	 */
	void resize(size_t width);

	/**
	 * @brief Checks if an enabled stat is available in the current platform
	 * @param index The stat index
	 * @return True if the stat is available, false otherwise
	 */
	bool is_available(StatIndex index) const;

	/**
	 * @param index The stat index of the data requested
	 * @return The data of the specified stat
	 */
	const std::vector<float> &get_data(StatIndex index) const
	{
		return counters.at(index);
	};

	/**
	 * @return The enabled stats
	 */
	const std::set<StatIndex> &get_enabled_stats() const
	{
		return enabled_stats;
	}

	/**
	 * @brief Update statistics, must be called after every frame
	 */
	void update();

  private:
	/// Stats to be enabled
	std::set<StatIndex> enabled_stats;

	/// Mapping of stats to their availability and value getters
	StatDataMap stat_data;

	/// Time of previous frame
	std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;

	/// Circular buffers for counter data
	std::map<StatIndex, std::vector<float>> counters{};

	/// Profiler to gather CPU and GPU performance data
	std::unique_ptr<hwcpipe::HWCPipe> hwcpipe{};
};

}        // namespace vkb
