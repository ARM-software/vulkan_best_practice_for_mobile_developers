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

#include "rendering/graphics_pipeline_state.h"
#include "rendering/render_frame.h"
#include "rendering/render_target.h"
#include "resource_cache.h"

namespace vkb
{
/**
 * @brief RenderContext acts as a frame manager for the sample, with a lifetime that is the
 * same as that of the Application itself. It acts as a container for RenderFrame objects,
 * swapping between them (begin_frame, end_frame) and forwarding requests for Vulkan resources
 * to the active frame. Note that it's guaranteed that there is always an active frame.
 * More than one frame can be in-flight in the GPU, thus the need for per-frame resources.
 *
 * It requires a Device to be valid on creation and will take control of the Swapchain, so a
 * RenderFrame can be created for each Swapchain image.
 *
 * A RenderContext can be extended for headless mode (i.e. not presenting rendered images to
 * a display) by removing the swapchain part and overriding begin_frame and end_frame.
 */
class RenderContext : public NonCopyable
{
  public:
	RenderContext(std::unique_ptr<Swapchain> &&swapchain, RenderTarget::CreateFunc create_render_target = RenderTarget::DEFAULT_CREATE_FUNC);

	virtual ~RenderContext() = default;

	/**
	 * @brief begin_frame
	 * 
	 * @return VkSemaphore 
	 */
	VkSemaphore begin_frame();

	VkSemaphore submit(const Queue &queue, const CommandBuffer &command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_pipeline_stage);

	/**
	 * @brief Submits a command buffer related to a frame to a queue
	 */
	void submit(const Queue &queue, const CommandBuffer &command_buffer);

	/**
	 * @brief Waits a frame to finish its rendering
	 */
	void wait_frame();

	void end_frame(VkSemaphore semaphore);

	/**
	 * @brief An error should be raised if the frame is not active.
	 *        A frame is active after @ref begin_frame has been called.
	 * @return The current active frame
	 */
	RenderFrame &get_active_frame();

	/**
	 * @brief An error should be raised if a frame is active.
	 *        A frame is active after @ref begin_frame has been called.
	 * @return The previous frame
	 */
	RenderFrame &get_last_rendered_frame();

	/**
	 * @brief Requests a command buffer to the command pool of the active frame
	 *        A frame should be active at the moment of requesting it
	 * @return A command buffer related to the current active frame
	 */
	CommandBuffer &request_frame_command_buffer(const Queue &queue);

	VkSemaphore request_semaphore();

	Device &get_device();

	void update_swapchain(std::unique_ptr<Swapchain> &&new_swapchain);

	Swapchain &get_swapchain();

	VkExtent2D get_surface_extent();

  protected:
	VkExtent2D surface_extent;

	virtual void handle_surface_changes();

  private:
	Device &device;

	std::unique_ptr<Swapchain> swapchain;

	/// Current active frame index
	uint32_t active_frame_index{0};

	/// Whether a frame is active or not
	bool frame_active{false};

	std::vector<RenderFrame> frames;

	/// Queue to submit commands for rendering our frames
	const Queue &present_queue;

	RenderTarget::CreateFunc create_render_target = RenderTarget::DEFAULT_CREATE_FUNC;
};

}        // namespace vkb
