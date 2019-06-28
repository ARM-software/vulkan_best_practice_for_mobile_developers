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

#include "command_buffer.h"
#include "common/helpers.h"
#include "common/vk_common.h"
#include "core/swapchain.h"

namespace vkb
{
class Device;

class Queue : NonCopyable
{
  public:
	Queue(Device &device, uint32_t family_index, VkQueueFamilyProperties properties, VkBool32 can_present, uint32_t index);

	/// @brief Move construct
	Queue(Queue &&other);

	const Device &get_device() const;

	VkQueue get_handle() const;

	uint32_t get_family_index() const;

	uint32_t get_index() const;

	VkQueueFamilyProperties get_properties() const;

	VkBool32 support_present() const;

	VkResult submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const;

	VkResult submit(const CommandBuffer &command_buffer, VkFence fence) const;

	VkResult present(const VkPresentInfoKHR &present_infos) const;

	VkResult wait_idle() const;

  private:
	Device &device;

	VkQueue handle{VK_NULL_HANDLE};

	uint32_t family_index{0};

	uint32_t index{0};

	VkBool32 can_present{VK_FALSE};

	VkQueueFamilyProperties properties{};
};
}        // namespace vkb
