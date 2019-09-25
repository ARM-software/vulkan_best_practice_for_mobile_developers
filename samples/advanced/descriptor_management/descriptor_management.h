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

#include "rendering/render_pipeline.h"
#include "scene_graph/components/perspective_camera.h"
#include "vulkan_sample.h"

class DescriptorManagement : public vkb::VulkanSample
{
  public:
	DescriptorManagement();

	virtual bool prepare(vkb::Platform &platform) override;

	virtual ~DescriptorManagement() = default;

	virtual void update(float delta_time) override;

  private:
	/**
	  * @brief Struct that contains radio button labeling and the value
	  *        which is selected
	  */
	struct RadioButtonGroup
	{
		const char *              description;
		std::vector<const char *> options;
		int                       value;
	};

	RadioButtonGroup descriptor_caching{
	    "Descriptor set caching",
	    {"Disabled", "Enabled"},
	    0};

	RadioButtonGroup buffer_allocation{
	    "Single large VkBuffer",
	    {"Disabled", "Enabled"},
	    0};

	std::vector<RadioButtonGroup *> radio_buttons = {&descriptor_caching, &buffer_allocation};

	vkb::sg::PerspectiveCamera *camera{nullptr};

	virtual void draw_gui() override;
};

std::unique_ptr<vkb::VulkanSample> create_descriptor_management();
