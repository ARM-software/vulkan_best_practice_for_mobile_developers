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

#include "fence_pool.h"

#include "core/device.h"

namespace vkb
{
FencePool::FencePool(Device &device) :
    device{device}
{
}

FencePool::~FencePool()
{
	wait();
	reset();

	// Destroy all fences
	for (VkFence fence : fences)
	{
		vkDestroyFence(device.get_handle(), fence, nullptr);
	}

	fences.clear();
}

VkFence FencePool::request_fence()
{
	// Check if there is an available fence
	if (active_fence_count < fences.size())
	{
		return fences.at(active_fence_count++);
	}

	VkFence fence{VK_NULL_HANDLE};

	VkFenceCreateInfo create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

	VkResult result = vkCreateFence(device.get_handle(), &create_info, nullptr, &fence);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fence.");
	}

	fences.push_back(fence);

	active_fence_count++;

	return fences.back();
}

VkResult FencePool::wait(uint32_t timeout)
{
	if (active_fence_count < 1 || fences.empty())
	{
		return VK_SUCCESS;
	}

	return vkWaitForFences(device.get_handle(), active_fence_count, fences.data(), true, timeout);
}

VkResult FencePool::reset()
{
	if (active_fence_count < 1 || fences.empty())
	{
		return VK_SUCCESS;
	}

	VkResult result = vkResetFences(device.get_handle(), active_fence_count, fences.data());

	if (result != VK_SUCCESS)
	{
		return result;
	}

	active_fence_count = 0;

	return VK_SUCCESS;
}
}        // namespace vkb
