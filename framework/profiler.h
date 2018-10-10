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

#include <chrono>
#include <thread>
#include <vector>

#include <instrument.h>

namespace vkb
{
using MeasurementsMap = Instrument::MeasurementsMap;

/// @brief A data point of performance data
struct PerformanceDataPoint
{
	/// @brief The interval in milliseconds from last data point
	std::chrono::duration<double, std::milli> interval;

	/// @brief The measurements for this data point
	std::vector<MeasurementsMap> measurements;

	PerformanceDataPoint(const std::chrono::duration<double, std::milli> interv,
	                     const std::vector<MeasurementsMap> &            measurements = {}) :
	    interval(interv),
	    measurements(measurements)
	{}
};

typedef std::vector<PerformanceDataPoint> FramePerformanceData;

/// @brief Profiler class to gather performance data
class Profiler
{
  public:
	/// @brief Initializes the Profiler
	/// @param instruments List of instruments to profile
	/// @param buffer_size Size of the ring buffer holding frame data
	Profiler(std::vector<std::shared_ptr<Instrument>> &&instruments = {}, const size_t buffer_size = 8);

	/// @brief Destroys the Profiler
	~Profiler();

	/// @brief Adds instruments to the profiler
	/// @param instruments List of instruments
	void add_instruments(std::vector<std::shared_ptr<Instrument>> &instruments);

	/// @brief Starts recording performances
	void start();

	/// @brief Stops recording performances
	void stop();

	/// @brief Gathers performance data
	const FramePerformanceData &get_frame_data();

  private:
	/// @brief Starts recording performances of the GPU
	void start_mali_counter();

	/// @brief Starts recording performances of the CPU
	void start_pmu_counter();

	std::chrono::high_resolution_clock::time_point last_sample_time_;

	std::vector<std::shared_ptr<Instrument>> instruments_;

	std::vector<FramePerformanceData> frame_data_ring_buffer_;

	size_t current_frame_ = 0;
};

}        // namespace vkb
