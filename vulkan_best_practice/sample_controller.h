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
#include "vulkan_sample.h"

namespace vkb
{
class SampleController : public Application
{
  public:
	SampleController();

	virtual bool prepare(Platform &platform) override;

	virtual void update(float delta_time) override;

	virtual void finish() override;

	virtual void resize(const uint32_t width, const uint32_t height) override;

	virtual void input_event(const InputEvent &input_event) override;

  private:
	bool prepare_sample(std::vector<SampleInfo>::const_iterator sample);

	bool parse_arguments(const std::vector<std::string> &arguments);

	Platform *platform;

	std::unique_ptr<Application> active_sample;

	std::vector<SampleInfo>::const_iterator current_sample;

	bool automatic_demo_mode = true;

	bool skipped_first_frame = false;

	float sample_run_time_per_configuration = 10.0f;

	float elapsed_time = 0.0f;
};

}        // namespace vkb

std::unique_ptr<vkb::Application> create_sample_controller();
