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

#include "profiler.h"

#include "common.h"

#include <memory>

namespace vkb
{
Profiler::Profiler(std::vector<std::shared_ptr<Instrument>> &&instruments, const size_t buffer_size) :
    instruments_{std::move(instruments)},
    frame_data_ring_buffer_(buffer_size)
{
	start();
}

Profiler::~Profiler()
{
	stop();
}

void Profiler::add_instruments(std::vector<std::shared_ptr<Instrument>> &other_instruments)
{
	instruments_.insert(instruments_.end(), other_instruments.begin(), other_instruments.end());
}

void Profiler::start()
{
	stop();

	for (auto &instrument : instruments_)
	{
		instrument->start();
	}
}

void Profiler::stop()
{
	for (auto &instrument : instruments_)
	{
		instrument->stop();
	}
}

const FramePerformanceData &Profiler::get_frame_data()
{
	// Stop counters
	stop();

	auto                                      now     = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = now - last_sample_time_;

	assert(current_frame_ < frame_data_ring_buffer_.size() && "Index out of ring buffer bounds");
	frame_data_ring_buffer_[current_frame_].clear();

	std::vector<MeasurementsMap> measurements;
	for (auto &instrument : instruments_)
	{
		measurements.emplace_back(instrument->measurements());
	}

	frame_data_ring_buffer_[current_frame_].emplace_back(elapsed, measurements);

	auto old_frame = current_frame_;
	current_frame_ = (current_frame_ + 1) % frame_data_ring_buffer_.size();
	return frame_data_ring_buffer_[old_frame];
}

}        // namespace vkb
