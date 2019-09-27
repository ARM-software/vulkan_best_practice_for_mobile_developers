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

#pragma once

#include <chrono>

namespace vkb
{
/**
 * @brief Encapsulates basic usage of chrono, providing a means to calculate float
 *        durations between time points via function calls.
 */
class Timer
{
  public:
	using Seconds      = std::ratio<1>;
	using Milliseconds = std::ratio<1, 1000>;
	using Microseconds = std::ratio<1, 1000000>;
	using Nanoseconds  = std::ratio<1, 1000000000>;

	// Configure
	using Clock             = std::chrono::steady_clock;
	using DefaultResolution = Seconds;

	Timer();

	virtual ~Timer() = default;

	/**
	 * @brief Starts the timer, elapsed() now returns the duration since start()
	 */
	void start();

	/**
	 * @brief Laps the timer, elapsed() now returns the duration since the last lap()
	 */
	void lap();

	/**
	 * @brief Stops the timer, elapsed() now returns 0
	 * @return The total execution time between `start()` and `stop()`
	 */
	template <typename T = DefaultResolution>
	double stop()
	{
		if (!running)
		{
			return 0;
		}

		running       = false;
		lapping       = false;
		auto duration = std::chrono::duration<double, T>(Clock::now() - start_time);
		start_time    = Clock::now();
		lap_time      = Clock::now();

		return duration.count();
	}

	/**
	 * @brief Calculates the time difference between now and when the timer was started
	 *        if lap() was called, then between now and when the timer was last lapped
	 * @return The duration between the two time points (default in milliseconds)
	 */
	template <typename T = DefaultResolution>
	double elapsed()
	{
		if (!running)
		{
			return 0;
		}

		Clock::time_point start = start_time;

		if (lapping)
		{
			start = lap_time;
		}

		return std::chrono::duration<double, T>(Clock::now() - start).count();
	}

	/**
	 * @brief Calculates the time difference between now and the last time this function was called
	 * @return The duration between the two time points (default in seconds)
	 */
	template <typename T = DefaultResolution>
	double tick()
	{
		auto now      = Clock::now();
		auto duration = std::chrono::duration<double, T>(now - previous_tick);
		previous_tick = now;
		return duration.count();
	}

	/**
	 * @brief Check if the timer is running
	 */
	bool is_running() const;

  private:
	bool running{false};

	bool lapping{false};

	Clock::time_point start_time;

	Clock::time_point lap_time;

	Clock::time_point previous_tick;
};
}        // namespace vkb
