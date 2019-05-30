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

#include "gui.h"
#include "platform/application.h"
#include "render_context.h"
#include "scene_graph/scene.h"
#include "scene_graph/scripts/node_animation.h"
#include "stats.h"

#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace vkb
{
class VulkanSample : public Application
{
  public:
	VulkanSample();

	virtual ~VulkanSample();

	virtual bool prepare(Platform &platform) override;

	virtual void update(float delta_time) override;

	virtual void resize(const uint32_t width, const uint32_t height) override;

	virtual void input_event(const InputEvent &input_event) override;

	VkPhysicalDevice get_gpu(size_t i);

	VkSurfaceKHR get_surface();

	/** 
	 * @brief Loads the scene
	 * 
	 * @param path The path of the gltf file
	 */
	void load_scene(const std::string &path);

	RenderContext &get_render_context()
	{
		assert(render_context && "Render context is not valid");
		return *render_context;
	}

  protected:
	std::unique_ptr<Device> device{nullptr};

	std::unique_ptr<RenderContext> render_context{nullptr};

	sg::Scene scene;

	std::unique_ptr<Gui> gui{nullptr};

	std::unique_ptr<Stats> stats{nullptr};

	/**
	 * @brief Resets the stats view max values for high demanding configs
	 *        Should be overriden by the samples since they
	 *        know which configuration is resource demanding
	 */
	virtual void reset_stats_view(){};

	virtual void draw_swapchain_renderpass(CommandBuffer &command_buffer, const RenderTarget &render_target);

	/**
	 * @brief Draw scene meshes to the command buffer
	 *
	 * @param command_buffer The Vulkan command buffer
	 */
	virtual void draw_scene(CommandBuffer &command_buffer);

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
