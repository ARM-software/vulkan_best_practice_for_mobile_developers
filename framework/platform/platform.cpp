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

#include "platform.h"

#include <ctime>
#include <mutex>
#include <vector>

#include <spdlog/async_logger.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "application.h"
#include "common/logging.h"
#include "platform/filesystem.h"

namespace vkb
{
std::vector<std::string> Platform::arguments = {};

std::string Platform::external_storage_directory = "";

std::string Platform::temp_directory = "";

Platform::Platform()
{
}

bool Platform::initialize(std::unique_ptr<Application> &&app)
{
	assert(app && "Application is not valid");
	active_app = std::move(app);

	// Override initialize_sinks in the derived platforms
	auto sinks = get_platform_sinks();

	auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
	logger->set_pattern(LOGGER_FORMAT);
	spdlog::set_default_logger(logger);

	LOGI("Logger initialized");

	if (active_app->get_options().contains("--benchmark"))
	{
		benchmark_mode             = true;
		total_benchmark_frames     = active_app->get_options().get_int("--benchmark");
		remaining_benchmark_frames = total_benchmark_frames;
		active_app->set_benchmark_mode(true);
	}

	return true;
}

bool Platform::prepare()
{
	if (active_app)
	{
		return active_app->prepare(*this);
	}
	return false;
}

void Platform::run()
{
	if (benchmark_mode)
	{
		timer.start();

		if (remaining_benchmark_frames == 0)
		{
			LOGI("Benchmark completed in {} seconds (ran {} frames)", timer.stop(), total_benchmark_frames);
			close();
		}
	}

	if (active_app->is_focused() || active_app->is_benchmark_mode())
	{
		active_app->step();
		remaining_benchmark_frames--;
	}
}

void Platform::terminate(ExitCode code)
{
	if (active_app)
	{
		active_app->finish();
	}

	active_app.reset();
	spdlog::drop_all();
}

const std::string &Platform::get_external_storage_directory()
{
	return external_storage_directory;
}

const std::string &Platform::get_temp_directory()
{
	return temp_directory;
}

float Platform::get_dpi_factor() const
{
	return 1.0;
}

Application &Platform::get_app() const
{
	assert(active_app && "Application is not valid");
	return *active_app;
}

std::vector<std::string> &Platform::get_arguments()
{
	return Platform::arguments;
}

void Platform::set_arguments(const std::vector<std::string> &args)
{
	arguments = args;
}

void Platform::set_external_storage_directory(const std::string &dir)
{
	external_storage_directory = dir;
}

void Platform::set_temp_directory(const std::string &dir)
{
	temp_directory = dir;
}

std::vector<spdlog::sink_ptr> Platform::get_platform_sinks()
{
	return {};
}
}        // namespace vkb
