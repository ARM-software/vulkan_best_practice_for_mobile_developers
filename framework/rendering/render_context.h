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
#include "rendering/pipeline_state.h"
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
 * It requires a Device to be valid on creation, and will take control of a given Swapchain.
 *
 * For normal rendering (using a swapchain), the RenderContext can be created by passing in a
 * swapchain. A RenderFrame will then be created for each Swapchain image.
 *
 * For headless rendering (no swapchain), the RenderContext can be given a valid Device, and
 * a width and height. A single RenderFrame will then be created.
 */
class RenderContext
{
  public:
	/**
	 * @brief Constructor
	 * @param device A valid device
	 * @param surface A surface, VK_NULL_HANDLE if in headless mode
	 * @param window_width The width of the window where the surface was created
	 * @param window_height The height of the window where the surface was created
	 */
	RenderContext(Device &device, VkSurfaceKHR surface, uint32_t window_width, uint32_t window_height);

	RenderContext(const RenderContext &) = delete;

	RenderContext(RenderContext &&) = default;

	virtual ~RenderContext() = default;

	RenderContext &operator=(const RenderContext &) = delete;

	RenderContext &operator=(RenderContext &&) = delete;
	/**
	 * @brief Prepares the RenderFrames for rendering
	 * @param thread_count The number of threads in the application, necessary to allocate this many resource pools for each RenderFrame
	 * @param create_render_target_func A function delegate, used to create a RenderTarget
	 */
	void prepare(size_t thread_count = 1, RenderTarget::CreateFunc create_render_target_func = RenderTarget::DEFAULT_CREATE_FUNC);

	/**
	 * @brief Updates the swapchains extent, if a swapchain exists
	 * @param extent The width and height of the new swapchain images
	 */
	void update_swapchain(const VkExtent2D &extent);

	/**
	 * @brief Updates the swapchains image count, if a swapchain exists
	 * @param image_count The amount of images in the new swapchain
	 */
	void update_swapchain(const uint32_t image_count);

	/**
	 * @brief Updates the swapchains image usage, if a swapchain exists
	 * @param image_usage_flags The usage flags the new swapchain images will have
	 */
	void update_swapchain(const std::set<VkImageUsageFlagBits> &image_usage_flags);

	/**
	 * @brief Updates the swapchains extent and surface transform, if a swapchain exists
	 * @param extent The width and height of the new swapchain images
	 * @param transform The surface transform flags
	 */
	void update_swapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

	/**
	 * @brief Recreates the RenderFrames, called after every update
	 */
	void recreate();

	/**
	 * @returns True if a valid swapchain exists in the RenderContext
	 */
	bool has_swapchain();

	/**
	 * @brief Prepares the next available frame for rendering
	 * @returns A valid command buffer to record commands to be submitted
	 */
	CommandBuffer &begin();

	/**
	 * @brief Submits the command buffer to the right queue
	 * @param command_buffer A command buffer containing recorded commands
	 */
	void submit(CommandBuffer &command_buffer);

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
	 * @brief An error should be raised if the frame is not active.
	 *        A frame is active after @ref begin_frame has been called.
	 * @return The current active frame index
	 */
	uint32_t get_active_frame_index();

	/**
	 * @brief An error should be raised if a frame is active.
	 *        A frame is active after @ref begin_frame has been called.
	 * @return The previous frame
	 */
	RenderFrame &get_last_rendered_frame();

	VkSemaphore request_semaphore();

	Device &get_device();

	Swapchain &get_swapchain();

	VkExtent2D get_surface_extent() const;

	uint32_t get_active_frame_index() const;

	std::vector<RenderFrame> &get_render_frames();

	void set_pre_transform(VkSurfaceTransformFlagBitsKHR pre_transform);

  protected:
	VkExtent2D surface_extent;

	/**
	 * @brief Handles surface changes, only applicable if the render_context makes use of a swapchain
	 */
	virtual void handle_surface_changes();

  private:
	Device &device;

	/// If swapchain exists, then this will be a present supported queue, else a graphics queue
	const Queue &queue;

	std::unique_ptr<Swapchain> swapchain;

	std::vector<RenderFrame> frames;

	VkSemaphore acquired_semaphore;

	bool prepared{false};

	/// Current active frame index
	uint32_t active_frame_index{0};

	/// Whether a frame is active or not
	bool frame_active{false};

	RenderTarget::CreateFunc create_render_target_func = RenderTarget::DEFAULT_CREATE_FUNC;

	VkSurfaceTransformFlagBitsKHR pre_transform{VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR};
};

}        // namespace vkb
