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

#include "application.h"

#include "common/logging.h"
#include "platform/platform.h"

namespace vkb
{
std::string Application::usage = "";

Application::Application() :
    name{"Sample Name"}
{
}

bool Application::prepare(Platform & /*platform*/)
{
	timer.start();
	return true;
}

void Application::step()
{
	auto delta_time = static_cast<float>(timer.tick<Timer::Seconds>());

	if (benchmark_mode)
	{
		// Fix the framerate to 60 FPS for benchmark mode
		delta_time = 0.01667f;
	}

	if (focus || benchmark_mode)
	{
		update(delta_time);
	}

	auto elapsed_time = static_cast<float>(timer.elapsed<Timer::Seconds>());

	frame_count++;

	if (elapsed_time > 0.5f)
	{
		fps        = frame_count / elapsed_time;
		frame_time = delta_time * 1000.0f;

		LOGI("FPS: {:.1f}", fps);

		frame_count = 0;
		timer.lap();
	}
}

void Application::finish()
{
	auto execution_time = timer.stop();
	LOGI("Closing App (Runtime: {:.1f})", execution_time);
}

void Application::resize(const uint32_t /*width*/, const uint32_t /*height*/)
{
}

void Application::input_event(const InputEvent &input_event)
{
	if (input_event.get_source() == EventSource::Keyboard)
	{
		const auto &key_event = static_cast<const KeyInputEvent &>(input_event);

		if (key_event.get_code() == KeyCode::Back ||
		    key_event.get_code() == KeyCode::Escape)
		{
			input_event.get_platform().close();
		}
	}
}

void Application::parse_options(const std::vector<std::string> &args)
{
	options = std::make_unique<Options>(usage, args);
}

void Application::set_usage(const std::string &usage)
{
	Application::usage = usage;
}

const std::string &Application::get_name() const
{
	return name;
}

void Application::set_name(const std::string &name_)
{
	name = name_;
}

bool Application::is_benchmark_mode() const
{
	return benchmark_mode;
}

DebugInfo &Application::get_debug_info()
{
	return debug_info;
}

const Options &Application::get_options()
{
	return *options;
}

void Application::set_benchmark_mode(bool benchmark_mode_)
{
	benchmark_mode = benchmark_mode_;
}

void Application::set_focus(bool flag)
{
	focus = flag;
}

bool Application::is_focused() const
{
	return focus;
}

}        // namespace vkb
