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

#include "queue.h"

#include "device.h"
#include "command_buffer.h"

namespace vkb
{
Queue::Queue(Device &device, uint32_t family_index, VkQueueFamilyProperties properties, VkBool32 can_present, uint32_t index) :
    device{device},
    family_index{family_index},
    index{index},
    can_present{can_present},
    properties{properties}
{
	vkGetDeviceQueue(device.get_handle(), family_index, index, &handle);
}

Queue::Queue(Queue &&other) :
    device{other.device},
    handle{other.handle},
    family_index{other.family_index},
    index{other.index},
    can_present{other.can_present},
    properties{other.properties}
{
	other.handle       = VK_NULL_HANDLE;
	other.family_index = {};
	other.properties   = {};
	other.can_present  = VK_FALSE;
	other.index        = 0;
}

const Device &Queue::get_device() const
{
	return device;
}

VkQueue Queue::get_handle() const
{
	return handle;
}

uint32_t Queue::get_family_index() const
{
	return family_index;
}

uint32_t Queue::get_index() const
{
	return index;
}

VkQueueFamilyProperties Queue::get_properties() const
{
	return properties;
}

VkBool32 Queue::support_present() const
{
	return can_present;
}

VkResult Queue::submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const
{
	return vkQueueSubmit(handle, to_u32(submit_infos.size()), submit_infos.data(), fence);
}

VkResult Queue::submit(const CommandBuffer &command_buffer, VkFence fence) const
{
	VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &command_buffer.get_handle();

	return submit({submit_info}, fence);
}

VkResult Queue::present(const VkPresentInfoKHR &present_info) const
{
	if (!can_present)
	{
		return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
	}

	return vkQueuePresentKHR(handle, &present_info);
}        // namespace vkb

VkResult Queue::wait_idle() const
{
	return vkQueueWaitIdle(handle);
}
}        // namespace vkb
