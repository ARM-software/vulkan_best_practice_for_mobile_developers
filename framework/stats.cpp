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

#include "stats.h"

#include "common.h"

namespace vkb
{
Stats::Stats(Profiler &profiler, const std::set<StatIndex> &enabled_stats, const size_t buffers_size) :
    enabled_stats(enabled_stats),
    cpu_cycles(buffers_size, 0),
    cpu_instructions(buffers_size, 0),
    cache_miss_ratio(buffers_size, 0),
    branch_miss_ratio(buffers_size, 0),
    gpu_cycles(buffers_size, 0),
    vertex_compute_cycles(buffers_size, 0),
    fragment_cycles(buffers_size, 0),
    l2_reads_lookups(buffers_size, 0),
    l2_ext_reads(buffers_size, 0),
    l2_writes_lookups(buffers_size, 0),
    l2_ext_writes(buffers_size, 0),
    l2_ext_read_stalls(buffers_size, 0),
    l2_ext_write_stalls(buffers_size, 0),
    l2_ext_read_beats(buffers_size, 0),
    l2_ext_write_beats(buffers_size, 0),
    frame_times(buffers_size, 0),
    profiler(profiler)
{
	assert(buffers_size >= 2 && "Buffers size should be greater than 2");
}

void Stats::resize(const size_t width)
{
	// The circular buffer size will be 1/16th of the width of the screen
	// which means every sixteen pixels represent one graph value
	size_t buffers_size = width >> 4;

	cpu_cycles.resize(buffers_size);
	cpu_cycles.shrink_to_fit();

	cpu_instructions.resize(buffers_size);
	cpu_instructions.shrink_to_fit();

	cache_miss_ratio.resize(buffers_size);
	cache_miss_ratio.shrink_to_fit();

	branch_miss_ratio.resize(buffers_size);
	branch_miss_ratio.shrink_to_fit();

	gpu_cycles.resize(buffers_size);
	gpu_cycles.shrink_to_fit();

	vertex_compute_cycles.resize(buffers_size);
	vertex_compute_cycles.shrink_to_fit();

	fragment_cycles.resize(buffers_size);
	fragment_cycles.shrink_to_fit();

	l2_reads_lookups.resize(buffers_size);
	l2_reads_lookups.shrink_to_fit();

	l2_ext_reads.resize(buffers_size);
	l2_ext_reads.shrink_to_fit();

	l2_writes_lookups.resize(buffers_size);
	l2_writes_lookups.shrink_to_fit();

	l2_ext_writes.resize(buffers_size);
	l2_ext_writes.shrink_to_fit();

	l2_ext_read_stalls.resize(buffers_size);
	l2_ext_read_stalls.shrink_to_fit();

	l2_ext_write_stalls.resize(buffers_size);
	l2_ext_write_stalls.shrink_to_fit();

	l2_ext_read_beats.resize(buffers_size);
	l2_ext_read_beats.shrink_to_fit();

	l2_ext_write_beats.resize(buffers_size);
	l2_ext_write_beats.shrink_to_fit();

	frame_times.resize(buffers_size);
	frame_times.shrink_to_fit();
}

bool Stats::is_available(const StatIndex index) const
{
	return enabled_stats.find(index) != enabled_stats.end() && available_stats.find(index) != available_stats.end();
}

const std::vector<float> &Stats::get_data(const StatIndex index) const
{
	switch (index)
	{
		case StatIndex::frame_times:
			return get_frame_times();
		case StatIndex::cpu_cycles:
			return get_cpu_cycles();
		case StatIndex::cpu_instructions:
			return get_cpu_instructions();
		case StatIndex::cache_miss_ratio:
			return get_cache_miss_ratio();
		case StatIndex::branch_miss_ratio:
			return get_branch_miss_ratio();
		case StatIndex::gpu_cycles:
			return get_gpu_cycles();
		case StatIndex::vertex_compute_cycles:
			return get_vertex_compute_cycles();
		case StatIndex::fragment_cycles:
			return get_fragment_cycles();
		case StatIndex::l2_ext_reads:
			return get_l2_ext_reads();
		case StatIndex::l2_ext_writes:
			return get_l2_ext_writes();
		case StatIndex::l2_ext_read_stalls:
			return get_l2_ext_read_stalls();
		case StatIndex::l2_ext_write_stalls:
			return get_l2_ext_write_stalls();
		case StatIndex::l2_ext_read_beats:
			return get_l2_ext_read_beats();
		case StatIndex::l2_ext_write_beats:
			return get_l2_ext_write_beats();
		default:
			throw std::runtime_error("Unimplemented stat index");
	}
}

void Stats::update()
{
	// Get current time
	auto current_time = std::chrono::high_resolution_clock::now();

	// Compute delta time
	float delta_time = std::chrono::duration<float, std::milli>(current_time - prev_time).count();

	// Overwrite previous time
	prev_time = current_time;

	// Shift values to the left to make space at the end and update frame times
	std::rotate(frame_times.begin(), frame_times.begin() + 1, frame_times.end());
	frame_times.back() = delta_time;

	const auto &frame_performance_data = profiler.get_frame_data();

	struct StatData
	{
		bool                is_cpu;
		std::string         name;
		bool                is_integer;
		bool                is_average;
		float *             stat_total;
		std::vector<float> *counter;
	};

	const std::map<StatIndex, StatData> stat_map{
	    {StatIndex::cpu_cycles, {true, "CPU cycles", true, false, &cpu_cycles_sum, &cpu_cycles}},
	    {StatIndex::cpu_instructions, {true, "CPU instructions", true, false, &cpu_instructions_sum, &cpu_instructions}},
	    {StatIndex::cache_miss_ratio, {true, "Cache miss ratio", false, true, &cache_miss_ratio_average, &cache_miss_ratio}},
	    {StatIndex::branch_miss_ratio, {true, "Branch miss ratio", false, true, &branch_miss_ratio_average, &branch_miss_ratio}},

	    {StatIndex::gpu_cycles, {false, "GPU cycles", true, false, &gpu_cycles_sum, &gpu_cycles}},
	    {StatIndex::vertex_compute_cycles, {false, "Vertex/compute cycles", true, false, &vertex_compute_cycles_sum, &vertex_compute_cycles}},
	    {StatIndex::fragment_cycles, {false, "Fragment cycles", true, false, &fragment_cycles_sum, &fragment_cycles}},
	    {StatIndex::l2_reads_lookups, {false, "L2 cache read lookups", true, false, &l2_reads_lookups_sum, &l2_reads_lookups}},
	    {StatIndex::l2_ext_reads, {false, "L2 cache external reads", true, false, &l2_ext_reads_sum, &l2_ext_reads}},
	    {StatIndex::l2_writes_lookups, {false, "L2 cache write lookups", true, false, &l2_writes_lookups_sum, &l2_writes_lookups}},
	    {StatIndex::l2_ext_writes, {false, "L2 cache external writes", true, false, &l2_ext_writes_sum, &l2_ext_writes}},
	    {StatIndex::l2_ext_read_stalls, {false, "L2 cache external read stalls", true, false, &l2_ext_read_stalls_sum, &l2_ext_read_stalls}},
	    {StatIndex::l2_ext_write_stalls, {false, "L2 cache external write stalls", true, false, &l2_ext_write_stalls_sum, &l2_ext_write_stalls}},
	    {StatIndex::l2_ext_read_beats, {false, "L2 cache external read beats", true, false, &l2_ext_read_beats_sum, &l2_ext_read_beats}},
	    {StatIndex::l2_ext_write_beats, {false, "L2 cache external write beats", true, false, &l2_ext_write_beats_sum, &l2_ext_write_beats}}};

	for (const auto &per_ms_data : frame_performance_data)
	{
		for (const auto &stat_index : enabled_stats)
		{
			auto stat_it = stat_map.find(stat_index);
			if (stat_it != std::end(stat_map))
			{
				const auto &stat_data = stat_it->second;

				// For non-averaged stats (e.g. bandwidth), divide by delta_time in seconds
				const float factor = stat_data.is_average ? 1.0f / frame_performance_data.size() : 1000.0f / delta_time;

				*stat_data.stat_total = 0.0f;

				for (auto &measurements : per_ms_data.measurements)
				{
					auto measurement_it = measurements.find(stat_data.name);
					if (measurement_it != std::end(measurements))
					{
						if (stat_data.is_integer)
						{
							*stat_data.stat_total += measurement_it->second.value().v.integer * factor;
						}
						else
						{
							*stat_data.stat_total += measurement_it->second.value().v.floating_point * factor;
						}

						available_stats.insert(stat_index);
					}
				}
			}
		}
	}

	for (const auto &stat_index : enabled_stats)
	{
		auto it = stat_map.find(stat_index);
		if (it != std::end(stat_map))
		{
			auto &stat_data = it->second;
			auto *counter   = stat_data.counter;

			if (counter->size() == counter->capacity())
			{
				// Shift values to the left to make space at the end and update counters
				std::rotate(counter->begin(), counter->begin() + 1, counter->end());
			}
			// Use an exponential moving average to smooth values
			const float alpha = 0.2f;
			counter->back()   = *stat_data.stat_total * alpha + *(counter->end() - 2) * (1.0f - alpha);
		}
	}
}

}        // namespace vkb
