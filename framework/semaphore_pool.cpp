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

#include "semaphore_pool.h"

#include "core/device.h"

namespace vkb
{
SemaphorePool::SemaphorePool(Device &device) :
    device{device}
{
}

SemaphorePool::~SemaphorePool()
{
	reset();

	// Destroy all semaphores
	for (VkSemaphore semaphore : semaphores)
	{
		vkDestroySemaphore(device.get_handle(), semaphore, nullptr);
	}

	semaphores.clear();
}

VkSemaphore SemaphorePool::request_semaphore()
{
	// Check if there is an available semaphore
	if (active_semaphore_count < semaphores.size())
	{
		return semaphores.at(active_semaphore_count++);
	}

	VkSemaphore semaphore{VK_NULL_HANDLE};

	VkSemaphoreCreateInfo create_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

	VkResult result = vkCreateSemaphore(device.get_handle(), &create_info, nullptr, &semaphore);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create semaphore.");
	}

	semaphores.push_back(semaphore);

	active_semaphore_count++;

	return semaphore;
}

void SemaphorePool::reset()
{
	active_semaphore_count = 0;
}

uint32_t SemaphorePool::get_active_semaphore_count() const
{
	return active_semaphore_count;
}
}        // namespace vkb
