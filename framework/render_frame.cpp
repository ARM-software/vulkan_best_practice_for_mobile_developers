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

#include "render_frame.h"

namespace vkb
{
const RenderFrame::CreateFunc RenderFrame::DEFAULT_CREATE_FUNC =
    [](Device &device, core::Image &&swapchain_image) {
	    return std::make_unique<RenderFrame>(device, std::move(swapchain_image));
    };

RenderFrame::RenderFrame(Device &device, core::Image &&swapchain_image) :
    device{device},
    fence_pool{device},
    semaphore_pool{device}
{
	update_render_target(std::move(swapchain_image));
}

RenderFrame::~RenderFrame()
{
	reset();
}

void RenderFrame::update_render_target(core::Image &&swapchain_image)
{
	core::Image depth_image{device, swapchain_image.get_extent(),
	                        VK_FORMAT_D32_SFLOAT,
	                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	                        VMA_MEMORY_USAGE_GPU_ONLY};

	std::vector<core::Image> main_images;
	main_images.push_back(std::move(swapchain_image));
	main_images.push_back(std::move(depth_image));

	swapchain_render_target = std::make_unique<RenderTarget>(device, std::move(main_images));
}

void RenderFrame::reset()
{
	fence_pool.wait();

	fence_pool.reset();

	for (auto &command_pool : command_pools)
	{
		command_pool.second.reset();
	}

	semaphore_pool.reset();
}

CommandPool &RenderFrame::get_command_pool(const Queue &queue)
{
	auto command_pool_it = command_pools.find(queue.get_family_index());

	if (command_pool_it != command_pools.end())
	{
		return command_pool_it->second;
	}

	auto res_ins_it = command_pools.emplace(queue.get_family_index(), CommandPool{device, queue.get_family_index()});

	if (!res_ins_it.second)
	{
		throw std::runtime_error("Failed to insert command pool");
	}

	command_pool_it = res_ins_it.first;

	return command_pool_it->second;
}

FencePool &RenderFrame::get_fence_pool()
{
	return fence_pool;
}

SemaphorePool &RenderFrame::get_semaphore_pool()
{
	return semaphore_pool;
}

const RenderTarget &RenderFrame::get_render_target() const
{
	return *swapchain_render_target;
}
}        // namespace vkb
