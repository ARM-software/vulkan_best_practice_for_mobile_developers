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

#include <functional>
#include <future>
#include <list>
#include <thread>

#include "concurrent_queue.h"

namespace vkb
{
class ThreadPool
{
  public:
	using Task = std::function<void()>;

	ThreadPool(uint32_t thread_count = std::thread::hardware_concurrency());

	~ThreadPool();

	template <class F, class... Arg>
	std::shared_future<void> run(F &&func, Arg &&... args)
	{
		return dispatch(std::bind(std::forward<F>(func),
		                          std::forward<Arg>(args)...));
	}

	std::shared_future<void> dispatch(const Task &task);

	// Cancel all pending tasks
	void clear();

	// Wait on all threads to complete
	void wait();

	// Create worker threads for the pool
	void start(uint32_t thread_count = std::thread::hardware_concurrency());

	// Cancel all pending tasks and wait thread to complete current tasks
	// Thread pool joins all worker threads.
	void stop();

  private:
	std::list<std::thread> worker_threads;

	ConcurrentQueue<std::packaged_task<void()>> pending_tasks;

	std::atomic<uint32_t> active_threads{0u};

	std::mutex threads_complete_mutex;

	std::condition_variable threads_complete_condition;

	void worker_main();
};
}        // namespace vkb
