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
Stats::Stats(const std::set<StatIndex> &enabled_stats, const size_t buffer_size) :
    hwcpipe(std::make_unique<hwcpipe::HWCPipe>()),
    enabled_stats(enabled_stats)
{
	assert(buffer_size >= 2 && "Buffers size should be greater than 2");

	for (const auto &stat : enabled_stats)
	{
		counters[stat] = std::vector<float>(buffer_size, 0);
	}

	stat_data = {
	    {StatIndex::frame_times, {StatScaling::None}},
	    {StatIndex::cpu_cycles, {hwcpipe::CpuCounter::Cycles}},
	    {StatIndex::cpu_instructions, {hwcpipe::CpuCounter::Instructions}},
	    {StatIndex::cache_miss_ratio, {hwcpipe::CpuCounter::CacheMisses}},
	    {StatIndex::branch_miss_ratio, {hwcpipe::CpuCounter::BranchMisses}},
	    {StatIndex::gpu_cycles, {hwcpipe::GpuCounter::GpuCycles}},
	    {StatIndex::vertex_compute_cycles, {hwcpipe::GpuCounter::VertexComputeCycles}},
	    {StatIndex::fragment_cycles, {hwcpipe::GpuCounter::FragmentCycles}},
	    {StatIndex::l2_reads_lookups, {hwcpipe::GpuCounter::CacheReadLookups}},
	    {StatIndex::l2_ext_reads, {hwcpipe::GpuCounter::ExternalMemoryReadAccesses}},
	    {StatIndex::l2_writes_lookups, {hwcpipe::GpuCounter::CacheWriteLookups}},
	    {StatIndex::l2_ext_writes, {hwcpipe::GpuCounter::ExternalMemoryWriteAccesses}},
	    {StatIndex::l2_ext_read_stalls, {hwcpipe::GpuCounter::ExternalMemoryReadStalls}},
	    {StatIndex::l2_ext_write_stalls, {hwcpipe::GpuCounter::ExternalMemoryWriteStalls}},
	    {StatIndex::l2_ext_read_bytes, {hwcpipe::GpuCounter::ExternalMemoryReadBytes}},
	    {StatIndex::l2_ext_write_bytes, {hwcpipe::GpuCounter::ExternalMemoryWriteBytes}},
	};
}

void Stats::resize(const size_t width)
{
	// The circular buffer size will be 1/16th of the width of the screen
	// which means every sixteen pixels represent one graph value
	size_t buffers_size = width >> 4;

	for (auto &counter : counters)
	{
		counter.second.resize(buffers_size);
		counter.second.shrink_to_fit();
	}
}

bool Stats::is_available(const StatIndex index) const
{
	const auto &data = stat_data.find(index);
	if (data == stat_data.end())
	{
		return false;
	}

	switch (data->second.type)
	{
		case StatType::Cpu:
		{
			if (hwcpipe->cpu_profiler())
			{
				const auto &cpu_supp = hwcpipe->cpu_profiler()->supported_counters();
				return cpu_supp.find(data->second.cpu_counter) != cpu_supp.end();
			}
			break;
		}
		case StatType::Gpu:
		{
			if (hwcpipe->gpu_profiler())
			{
				const auto &gpu_supp = hwcpipe->gpu_profiler()->supported_counters();
				return gpu_supp.find(data->second.gpu_counter) != gpu_supp.end();
			}
			break;
		}
		case StatType::Other:
		{
			return true;
		}
	}

	return false;
}

void Stats::update()
{
	// Get current time
	auto current_time = std::chrono::high_resolution_clock::now();

	// Compute delta time
	float delta_time = std::chrono::duration<float>(current_time - prev_time).count();

	// Overwrite previous time
	prev_time = current_time;

	const auto measurements = hwcpipe->sample();

	for (auto &c : counters)
	{
		auto &counter = c.second;

		const auto &data = stat_data.find(c.first);
		if (data == stat_data.end())
		{
			continue;
		}

		float measurement = 0;
		switch (data->second.type)
		{
			case StatType::Cpu:
			{
				if (measurements.cpu)
				{
					const auto &cpu_res = measurements.cpu->find(data->second.cpu_counter);
					if (cpu_res != measurements.cpu->end())
					{
						measurement = cpu_res->second.get<float>();
					}
				}
				break;
			}
			case StatType::Gpu:
			{
				if (measurements.gpu)
				{
					const auto &gpu_res = measurements.gpu->find(data->second.gpu_counter);
					if (gpu_res != measurements.gpu->end())
					{
						measurement = gpu_res->second.get<float>();
					}
				}
				break;
			}
			case StatType::Other:
			{
				switch (c.first)
				{
					case StatIndex::frame_times:
						measurement = delta_time * 1000;        // ms
						break;
					default:
						break;
				}
				break;
			}
		}

		if (data->second.scaling == StatScaling::ByDeltaTime)
		{
			measurement /= delta_time;
		}

		if (counter.size() == counter.capacity())
		{
			// Shift values to the left to make space at the end and update counters
			std::rotate(counter.begin(), counter.begin() + 1, counter.end());
		}

		// Use an exponential moving average to smooth values
		const float alpha = 0.2f;
		counter.back()    = measurement * alpha + *(counter.end() - 2) * (1.0f - alpha);
	}
}

}        // namespace vkb
