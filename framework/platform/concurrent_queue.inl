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

#include "concurrent_queue.h"
#pragma once

namespace vkb
{
template <class T>
inline bool ConcurrentQueue<T>::empty()
{
	std::lock_guard<std::mutex> lock(mutex);

	return queue.empty();
}

template <class T>
inline void ConcurrentQueue<T>::clear()
{
	std::lock_guard<std::mutex> lock(mutex);

	// Remove all items in the queue
	while (!queue.empty())
	{
		queue.pop();
	}

	condition.notify_all();
}

template <class T>
inline void ConcurrentQueue<T>::push(const T &item)
{
	std::unique_lock<std::mutex> lock(mutex);

	// Add new item to the queue
	queue.push(item);

	condition.notify_one();
}

template <class T>
inline void ConcurrentQueue<T>::push(T &&item)
{
	std::unique_lock<std::mutex> lock(mutex);

	// Add new item to the queue
	queue.push(std::move(item));

	condition.notify_one();
}

template <class T>
inline bool ConcurrentQueue<T>::pop(T &item)
{
	std::unique_lock<std::mutex> lock(mutex);

	// Wait for an item to exists in the queue or the queue to be invalidated
	condition.wait(lock, [&](void) { return !queue.empty() || !valid; });

	// Get item only if queue is valid
	if (!valid)
	{
		return false;
	}

	item = std::move(queue.front());

	queue.pop();

	return true;
}

template <class T>
inline void ConcurrentQueue<T>::set_valid(bool flag)
{
	valid = flag;

	condition.notify_all();
}
}        // namespace vkb
