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

#include "thread_pool.h"

namespace vkb
{
ThreadPool::ThreadPool(uint32_t thread_count)
{
	start(thread_count);
}

ThreadPool::~ThreadPool()
{
	stop();
}

std::shared_future<void> ThreadPool::dispatch(const Task &task)
{
	std::packaged_task<void()> packaged_task(std::move(task));

	auto future = packaged_task.get_future();

	pending_tasks.push(std::move(packaged_task));

	return future.share();
}

void ThreadPool::clear()
{
	pending_tasks.clear();
}

void ThreadPool::wait()
{
	std::unique_lock<std::mutex> lock(threads_complete_mutex);
	threads_complete_condition.wait(lock,
	                                [&]() {
		                                return active_threads == 0 && pending_tasks.empty();
	                                });
}

void ThreadPool::start(uint32_t thread_count)
{
	// Mark queue as valid so worker threads can wait for tasks
	pending_tasks.set_valid(true);

	for (auto i = 0U; i < thread_count; ++i)
	{
		// Allocate new worker thread
		worker_threads.push_back(std::thread(&ThreadPool::worker_main, this));
	}
}

void ThreadPool::stop()
{
	pending_tasks.clear();

	// Stop worker threads wait for tasks process
	pending_tasks.set_valid(false);

	wait();

	// Join all worker threads
	while (!worker_threads.empty())
	{
		auto &thread = worker_threads.front();

		thread.join();

		worker_threads.pop_front();
	}
}

void ThreadPool::worker_main()
{
	while (true)
	{
		std::packaged_task<void()> packaged_task;

		// Get pending task from queue.
		// Stop if queue is invalidated
		if (!pending_tasks.pop(packaged_task))
		{
			break;
		}

		// Increment the number of active threads
		++active_threads;

		// Run the task.
		packaged_task();

		// Decrement the number of active threads
		--active_threads;

		// Notify all threads waiting on pending task to complete
		threads_complete_condition.notify_all();
	}
}
}        // namespace vkb
