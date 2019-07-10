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
#include "platform/file.h"

inline std::tm thread_safe_time(const std::time_t time)
{
	std::tm result;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	localtime_s(&result, &time);
#else
	std::mutex                  mtx;
	std::lock_guard<std::mutex> lock(mtx);
	result = *std::localtime(&time);
#endif
	return result;
}

namespace vkb
{
Platform::Platform() :
    arguments{""}
{
}

bool Platform::initialize(std::unique_ptr<Application> &&app)
{
	assert(app && "Application is not valid");
	active_app = std::move(app);
	return active_app->prepare(*this);
}

void Platform::terminate(ExitCode code)
{
	if (active_app)
	{
		active_app->finish();
	}

	active_app.reset();
	spdlog::drop_all();

	if (code == ExitCode::Success)
	{
		std::remove(log_output.c_str());
	}
}

void Platform::prepare_logger(std::vector<spdlog::sink_ptr> sinks)
{
	char        timestamp[80];
	std::time_t time = std::time(0);
	std::tm     now  = thread_safe_time(time);
	std::strftime(timestamp, 80, "%G-%m-%d_%H-%M-%S_log.txt", &now);
	log_output = vkb::file::Path::logs() + std::string(timestamp);

	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_output, true));

	auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
	logger->set_pattern(LOGGER_FORMAT);
	spdlog::set_default_logger(logger);

	LOGI("Logger Initialized");
}

const ArgumentParser &Platform::get_arguments()
{
	return arguments;
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

void Platform::parse_arguments(const std::string &argument_string)
{
	arguments = ArgumentParser{argument_string};
}

std::string &Platform::get_log_output_path()
{
	return log_output;
};

}        // namespace vkb
