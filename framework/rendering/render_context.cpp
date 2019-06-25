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

#include "rendering/render_context.h"

namespace vkb
{
RenderContext::RenderContext(std::unique_ptr<Swapchain> &&s, RenderTarget::CreateFunc create_rt) :
    device{s->get_device()},
    swapchain{std::move(s)},
    present_queue{device.get_queue_by_present(0)},
    create_render_target{create_rt}
{
	// Prepare is an expensive operation, we want to be sure it happens only once
	assert(frames.empty());

	// Create the render frames
	device.wait_idle();
	auto &     swapchain_extent = swapchain->get_extent();
	VkExtent3D extent{swapchain_extent.width, swapchain_extent.height, 1};

	for (auto &image_handle : swapchain->get_images())
	{
		auto swapchain_image = core::Image{
		    device, image_handle,
		    extent,
		    swapchain->get_format(),
		    swapchain->get_usage()};
		auto render_target = create_render_target(std::move(swapchain_image));
		frames.emplace_back(RenderFrame{device, std::move(render_target)});
	}
}

VkSemaphore RenderContext::begin_frame()
{
	handle_surface_changes();
	assert(!frame_active && "Frame is still active, please call end_frame");

	auto &prev_frame = frames.at(active_frame_index);

	auto fence = prev_frame.get_fence_pool().request_fence();

	auto aquired_semaphore = prev_frame.get_semaphore_pool().request_semaphore();

	auto result = swapchain->acquire_next_image(active_frame_index, aquired_semaphore, fence);

	if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		handle_surface_changes();

		result = swapchain->acquire_next_image(active_frame_index, aquired_semaphore, fence);
	}

	if (result != VK_SUCCESS)
	{
		prev_frame.reset();

		return VK_NULL_HANDLE;
	}

	// Now the frame is active again
	frame_active = true;

	wait_frame();

	return aquired_semaphore;
}

VkSemaphore RenderContext::submit(const Queue &queue, const CommandBuffer &command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_pipeline_stage)
{
	RenderFrame &frame = get_active_frame();

	VkSemaphore signal_semaphore = frame.get_semaphore_pool().request_semaphore();

	VkCommandBuffer cmd_buf = command_buffer.get_handle();

	VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};

	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &cmd_buf;
	submit_info.waitSemaphoreCount   = 1;
	submit_info.pWaitSemaphores      = &wait_semaphore;
	submit_info.pWaitDstStageMask    = &wait_pipeline_stage;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &signal_semaphore;

	VkFence fence = frame.get_fence_pool().request_fence();

	queue.submit({submit_info}, fence);

	return signal_semaphore;
}

void RenderContext::submit(const Queue &queue, const CommandBuffer &command_buffer)
{
	RenderFrame &frame = get_active_frame();

	VkCommandBuffer cmd_buf = command_buffer.get_handle();

	VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &cmd_buf;

	VkFence fence = frame.get_fence_pool().request_fence();

	queue.submit({submit_info}, fence);
}

void RenderContext::wait_frame()
{
	RenderFrame &frame = get_active_frame();
	frame.reset();
}

void RenderContext::end_frame(VkSemaphore semaphore)
{
	assert(frame_active && "Frame is not active, please call begin_frame");

	VkSwapchainKHR vk_swapchain = swapchain->get_handle();

	VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &semaphore;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = &vk_swapchain;
	present_info.pImageIndices      = &active_frame_index;

	VkResult result = present_queue.present(present_info);

	if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		handle_surface_changes();
	}
	// Frame is not active anymore
	frame_active = false;
}

RenderFrame &RenderContext::get_active_frame()
{
	assert(frame_active && "Frame is not active, please call begin_frame");
	return frames.at(active_frame_index);
}

RenderFrame &RenderContext::get_last_rendered_frame()
{
	assert(!frame_active && "Frame is still active, please call end_frame");
	return frames.at(active_frame_index);
}

CommandBuffer &RenderContext::request_frame_command_buffer(const Queue &queue)
{
	RenderFrame &frame = get_active_frame();

	return frame.get_command_pool(queue).request_command_buffer();
}

VkSemaphore RenderContext::request_semaphore()
{
	RenderFrame &frame = get_active_frame();
	return frame.get_semaphore_pool().request_semaphore();
}

Device &RenderContext::get_device()
{
	return device;
}

void RenderContext::update_swapchain(std::unique_ptr<Swapchain> &&new_swapchain)
{
	device.get_resource_cache().clear_framebuffers();

	swapchain = std::move(new_swapchain);

	VkExtent2D swapchain_extent = swapchain->get_extent();
	VkExtent3D extent{swapchain_extent.width, swapchain_extent.height, 1};

	auto frame_it = frames.begin();

	for (auto &image_handle : swapchain->get_images())
	{
		core::Image swapchain_image{device, image_handle,
		                            extent,
		                            swapchain->get_format(),
		                            swapchain->get_usage()};

		auto render_target = create_render_target(std::move(swapchain_image));
		frame_it->update_render_target(std::move(render_target));

		++frame_it;
	}
}

void RenderContext::handle_surface_changes()
{
	VkSurfaceCapabilitiesKHR surface_properties;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_physical_device(),
	                                                   swapchain->get_surface(),
	                                                   &surface_properties));

	if (surface_properties.currentExtent.width != surface_extent.width ||
	    surface_properties.currentExtent.height != surface_extent.height)
	{
		// Recreate swapchain
		device.wait_idle();

		auto new_swapchain = std::make_unique<vkb::Swapchain>(
		    get_swapchain(),
		    surface_properties.currentExtent);

		update_swapchain(std::move(new_swapchain));

		surface_extent = surface_properties.currentExtent;
	}
}

Swapchain &RenderContext::get_swapchain()
{
	assert(swapchain && "Swapchain is not valid");
	return *swapchain;
}

VkExtent2D RenderContext::get_surface_extent()
{
	return surface_extent;
}
}        // namespace vkb
