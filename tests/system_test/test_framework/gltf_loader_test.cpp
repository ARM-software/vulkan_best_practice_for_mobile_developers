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

#include "gltf_loader_test.h"

#include "gltf_loader.h"
#include "gui.h"
#include "platform/filesystem.h"
#include "platform/platform.h"
#include "rendering/subpasses/forward_subpass.h"
#include "stats.h"
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

namespace vkbtest
{
GLTFLoaderTest::GLTFLoaderTest(const std::string &scene_path) :
    scene_path{scene_path}
{
}

bool GLTFLoaderTest::prepare(vkb::Platform &platform)
{
	if (!VulkanTest::prepare(platform))
	{
		return false;
	}

	load_scene(scene_path);

	scene->clear_components<vkb::sg::Light>();

	vkb::add_point_light(get_scene(), {500.0f, 1550.0f, 0.0f});

	auto camera_node = scene->find_node("main_camera");

	if (!camera_node)
	{
		LOGW("Camera node not found. Looking for `default_camera` node.");

		camera_node = scene->find_node("default_camera");
	}

	auto &camera = camera_node->get_component<vkb::sg::Camera>();

	vkb::ShaderSource vert_shader(vkb::fs::read_shader("base.vert"));
	vkb::ShaderSource frag_shader(vkb::fs::read_shader("base.frag"));

	auto scene_subpass = std::make_unique<vkb::ForwardSubpass>(get_render_context(), std::move(vert_shader), std::move(frag_shader), *scene, camera);

	auto render_pipeline = vkb::RenderPipeline();
	render_pipeline.add_subpass(std::move(scene_subpass));

	VulkanSample::set_render_pipeline(std::move(render_pipeline));

	return true;
}

}        // namespace vkbtest
