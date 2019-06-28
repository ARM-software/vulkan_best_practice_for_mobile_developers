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

#include "common/helpers.h"
#include "common/logging.h"
#include "common/vk_common.h"
#include "core/command_buffer.h"
#include "core/command_pool.h"
#include "core/descriptor_set.h"
#include "core/descriptor_set_layout.h"
#include "core/framebuffer.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/queue.h"
#include "core/render_pass.h"
#include "core/shader_module.h"
#include "core/swapchain.h"
#include "fence_pool.h"
#include "rendering/graphics_pipeline_state.h"
#include "rendering/render_target.h"
#include "resource_cache.h"

namespace vkb
{
class Device : public NonCopyable
{
  public:
	Device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, std::vector<const char *> extensions = {}, const VkPhysicalDeviceFeatures &features = {});

	~Device();

	VkPhysicalDevice get_physical_device() const;

	const VkPhysicalDeviceFeatures &get_features() const;

	VkDevice get_handle() const;

	VmaAllocator get_memory_allocator() const;

	const VkPhysicalDeviceProperties &get_properties() const;

	const VkFormatProperties get_format_properties(VkFormat format) const;

	const Queue &get_queue(uint32_t queue_family_index, uint32_t queue_index);

	const Queue &get_queue_by_flags(VkQueueFlags queue_flags, uint32_t queue_index);

	const Queue &get_queue_by_present(uint32_t queue_index);

	/**
	 * @return The command pool
	 */
	CommandPool &get_command_pool()
	{
		return *command_pool;
	}

	/**
	 * @brief Requests a command buffer from the general command_pool
	 * @return A new command buffer
	 */
	CommandBuffer &request_command_buffer();

	/**
	 * @return The fence pool
	 */
	FencePool &get_fence_pool()
	{
		return *fence_pool;
	}

	/**
	 * @brief Requests a fence to the fence pool
	 * @return A vulkan fence
	 */
	VkFence request_fence();

	VkResult wait_idle();

	ResourceCache &get_resource_cache();

  private:
	VkPhysicalDevice physical_device{VK_NULL_HANDLE};

	VkPhysicalDeviceFeatures features{};

	VkSurfaceKHR surface{VK_NULL_HANDLE};

	uint32_t queue_family_count{0};

	VkDevice handle{VK_NULL_HANDLE};

	VmaAllocator memory_allocator{VK_NULL_HANDLE};

	VkPhysicalDeviceProperties properties;

	std::vector<std::vector<Queue>> queues;

	/// A command pool associated to the primary queue
	std::unique_ptr<CommandPool> command_pool;

	/// A fence pool associated to the primary queue
	std::unique_ptr<FencePool> fence_pool;

	ResourceCache resource_cache;
};
}        // namespace vkb
