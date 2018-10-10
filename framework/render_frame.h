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

#include "core/buffer.h"
#include "core/command_pool.h"
#include "core/device.h"
#include "core/image.h"
#include "core/queue.h"
#include "fence_pool.h"
#include "render_target.h"
#include "semaphore_pool.h"

namespace vkb
{
class RenderFrame : public NonCopyable
{
  public:
	using CreateFunc = std::function<std::unique_ptr<RenderFrame>(Device &, core::Image &&)>;

	static const CreateFunc DEFAULT_CREATE_FUNC;

	RenderFrame(Device &device, core::Image &&swapchain_image);

	virtual ~RenderFrame();

	RenderFrame(RenderFrame &&other) = default;

	void reset();

	Device &get_device()
	{
		return device;
	}

	CommandPool &get_command_pool(const Queue &queue);

	FencePool &get_fence_pool();

	SemaphorePool &get_semaphore_pool();

	void update_render_target(core::Image &&swapchain_image);

	const RenderTarget &get_render_target() const;

	std::unique_ptr<core::Buffer> gui_vertex_buffer;

	std::unique_ptr<core::Buffer> gui_index_buffer;

  private:
	Device &device;

	/// Commands pools associated to the frames
	std::map<uint32_t, CommandPool> command_pools;

	FencePool fence_pool;

	SemaphorePool semaphore_pool;

	std::unique_ptr<RenderTarget> swapchain_render_target;
};
}        // namespace vkb
