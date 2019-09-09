/* Copyright (c) 2018-2019, Arm Limited and Contributors
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

#include <memory>

#include "platform/application.h"
#include "samples.h"
#include "tests.h"
#include "vulkan_sample.h"

namespace vkb
{
using CreateAppFunc = std::function<std::unique_ptr<vkb::Application>()>;

class VulkanBestPractice : public Application
{
  public:
	VulkanBestPractice();

	virtual ~VulkanBestPractice() = default;

	virtual bool prepare(Platform &platform) override;

	virtual void update(float delta_time) override;

	virtual void finish() override;

	virtual void resize(const uint32_t width, const uint32_t height) override;

	virtual void input_event(const InputEvent &input_event) override;

	/** 
	 * @brief Prepares a sample or a test to be run under certain conditions
	 * @param run_info A struct containing the information needed to run
	 * @returns true if the preparation was a success, false if there was a failure
	 */
	bool prepare_active_app(CreateAppFunc create_app_func, const std::string &name, bool test, bool batch);

  private:
	/// Platform pointer
	Platform *platform;

	/// The actual sample that the vulkan best practices controls
	std::unique_ptr<Application> active_app{nullptr};

	/// The list of suitable samples to be run in conjunction with batch mode
	std::vector<SampleInfo> batch_mode_sample_list{};

	/// An iterator to the current batch mode sample info object
	std::vector<SampleInfo>::const_iterator batch_mode_sample_iter;

	/// If batch mode is enabled
	bool batch_mode{false};

	/// The first frame is skipped as we don't want to include the prepare time
	bool skipped_first_frame{false};

	/// The amount of time run per configuration for each sample
	float sample_run_time_per_configuration{3.0f};

	/// Used to calculate when the sample has exceeded the sample_run_time_per_configuration
	float elapsed_time{0.0f};
};

}        // namespace vkb

std::unique_ptr<vkb::Application> create_vulkan_best_practice();
