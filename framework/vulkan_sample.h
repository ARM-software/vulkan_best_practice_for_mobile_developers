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

#include "common/error.h"
#include "common/utils.h"
#include "common/vk_common.h"
#include "gui.h"
#include "platform/application.h"
#include "rendering/render_context.h"
#include "rendering/render_pipeline.h"
#include "scene_graph/node.h"
#include "scene_graph/scene.h"
#include "scene_graph/scripts/node_animation.h"
#include "stats.h"

namespace vkb
{
/**
 * @mainpage Overview of the framework
 *
 * @section initialization Initialization
 *
 * @subsection platform_init Platform initialization
 * The lifecycle of a Vulkan sample starts by instantiating the correct Platform
 * (e.g. WindowsPlatform) and then calling initialize() on it, which sets up
 * the windowing system and logging. Then it calls the parent Platform::initialize(),
 * which takes ownership of the active application an calls Application::prepare().
 *
 * @subsection sample_init Sample initialization
 * The preparation step is divided in two steps, one in VulkanSample and the other in the
 * specific sample, such as SurfaceRotation.
 * VulkanSample::prepare() contains functions that do not require customization,
 * including creating a Vulkan instance, the surface and getting physical devices.
 * The prepare() function for the specific sample completes the initialization, including:
 * - setting enabled Stats
 * - creating the Device
 * - creating the Swapchain
 * - creating the RenderContext (or child class)
 * - preparing the RenderContext
 * - loading the sg::Scene
 * - creating the RenderPipeline with ShaderModule (s)
 * - creating the sg::Camera
 * - creating the Gui
 *
 * @section frame_rendering Frame rendering
 *
 * @subsection update Update function
 * Rendering happens in the update() function. Each sample can override it, e.g.
 * to recreate the Swapchain in SwapchainImages when required by user input.
 * Typically a sample will then call VulkanSample::update().
 *
 * @subsection rendering Rendering
 * A series of steps are performed, some of which can be customized (it will be
 * highlighted when that's the case):
 *
 * - calling sg::Script::update() for all sg::Script (s)
 * - beginning a frame in RenderContext (does the necessary waiting on fences and
 *   acquires an core::Image)
 * - requesting a CommandBuffer
 * - updating Stats and Gui
 * - getting an active RenderTarget constructed by the factory function of the RenderFrame
 * - setting up barriers for color and depth, note that these are only for the default RenderTarget
 * - calling VulkanSample::draw_swapchain_renderpass (see below)
 * - setting up a barrier for the Swapchain transition to present
 * - submitting the CommandBuffer and end the Frame (present)
 *
 * @subsection draw_swapchain Draw swapchain renderpass
 * The function starts and ends a RenderPass which includes setting up viewport, scissors,
 * blend state (etc.) and calling draw_scene.
 * Note that RenderPipeline::draw is not virtual in RenderPipeline, but internally it calls
 * Subpass::draw for each Subpass, which is virtual and can be customized.
 *
 * @section framework_classes Main framework classes
 *
 * - RenderContext
 * - RenderFrame
 * - RenderTarget
 * - RenderPipeline
 * - ShaderModule
 * - ResourceCache
 * - BufferPool
 * - Core classes: Classes in vkb::core wrap Vulkan objects for indexing and hashing.
 */

class VulkanSample : public Application
{
  public:
	VulkanSample();

	virtual ~VulkanSample();

	/**
	 * @brief Additional sample initialization
	 */
	virtual bool prepare(Platform &platform) override;

	/**
	 * @brief Main loop sample events
	 */
	virtual void update(float delta_time) override;

	virtual void resize(const uint32_t width, const uint32_t height) override;

	virtual void input_event(const InputEvent &input_event) override;

	virtual void finish() override;

	/**
	 * @return A suitable GPU
	 */
	VkPhysicalDevice get_gpu();

	VkSurfaceKHR get_surface();

	/** 
	 * @brief Loads the scene
	 * 
	 * @param path The path of the glTF file
	 */
	void load_scene(const std::string &path);

	RenderContext &get_render_context();

	void set_render_pipeline(RenderPipeline &&render_pipeline);

	RenderPipeline &get_render_pipeline();

	Configuration &get_configuration();

	sg::Scene &get_scene();

  protected:
	std::unique_ptr<Device> device{nullptr};

	std::unique_ptr<RenderContext> render_context{nullptr};

	std::unique_ptr<sg::Scene> scene{nullptr};

	std::unique_ptr<Gui> gui{nullptr};

	std::unique_ptr<Stats> stats{nullptr};

	/**
	 * @brief Update scene
	 * @param delta_time
	 */
	void update_scene(float delta_time);

	/**
	 * @brief Update counter values
	 * @param delta_time
	 */
	void update_stats(float delta_time);

	/**
	 * @brief Update GUI
	 * @param delta_time
	 */
	void update_gui(float delta_time);

	/**
	 * @brief Sets up the necessary image memory barriers for all attachments
	 *        and calls draw_swapchain_renderpass
	 * @param command_buffer
	 * @param render_target
	 */
	void record_scene_rendering_commands(CommandBuffer &command_buffer, RenderTarget &render_target);

	/**
	 * @brief Get sample-specific instance layers.
	 * 
	 * @return Vector of additional instance layers. Default is empty vector.
	 */
	virtual std::vector<const char *> get_sample_additional_layers();

	/**
	 * @brief Resets the stats view max values for high demanding configs
	 *        Should be overriden by the samples since they
	 *        know which configuration is resource demanding
	 */
	virtual void reset_stats_view(){};

	/**
	 * @brief Record render pass for drawing the scene
	 */
	virtual void draw_swapchain_renderpass(CommandBuffer &command_buffer, RenderTarget &render_target);

	/**
	 * @brief Triggers rendering, it can be overriden by samples to specialize their rendering logic
	 * @param command_buffer The Vulkan command buffer
	 */
	virtual void render(CommandBuffer &command_buffer);

	/**
	 * @brief Samples should override this function to draw their interface
	 */
	virtual void draw_gui();

	/**
	 * @brief Updates the debug window, samples can override this to insert their own data elements
	 */
	virtual void update_debug_window();

	/**
	 * @brief Add free camera script to a node with a camera object.
	 *        Fallback to the default_camera if node not found.
	 *
	 * @param node_name The scene node name
	 *
	 * @return Node where the script was attached as component
	 */
	sg::Node &add_free_camera(const std::string &node_name);

	/**
	 * @brief Pipeline used for rendering, it should be set up by the concrete sample
	 */
	std::unique_ptr<RenderPipeline> render_pipeline{nullptr};

  private:
	static constexpr float STATS_VIEW_RESET_TIME{10.0f};        // 10 seconds

#if defined(VKB_DEBUG) || defined(VKB_VALIDATION_LAYERS)
	/// The debug report callback
	VkDebugReportCallbackEXT debug_report_callback{VK_NULL_HANDLE};
#endif

	/**
	 * @brief The Vulkan instance
	 */
	VkInstance instance{VK_NULL_HANDLE};

	/**
	 * @brief The Vulkan surface
	 */
	VkSurfaceKHR surface{VK_NULL_HANDLE};

	/**
	 * @brief The physical devices found on the machine
	 */
	std::vector<VkPhysicalDevice> gpus;

	/**
	 * @brief The configuration of the sample
	 */
	Configuration configuration{};

	/**
	 * @brief Create a Vulkan instance
	 *
	 * @param required_instance_extensions The required Vulkan instance extensions
	 * @param required_instance_layers The required Vulkan instance layers
	 *
	 * @return Vulkan instance object
	 */
	VkInstance create_instance(const std::vector<const char *> &required_instance_extensions = {},
	                           const std::vector<const char *> &required_instance_layers     = {});
};
}        // namespace vkb
