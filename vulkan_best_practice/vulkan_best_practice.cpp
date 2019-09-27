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

#include "vulkan_best_practice.h"

#include "common/logging.h"
#include "platform/platform.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include <jni.h>

extern "C"
{
	JNIEXPORT jobjectArray JNICALL
	    Java_com_arm_vulkan_1best_1practice_BPSampleActivity_getSamples(JNIEnv *env, jobject thiz)
	{
		jclass       c             = env->FindClass("com/arm/vulkan_best_practice/Sample");
		jmethodID    constructor   = env->GetMethodID(c, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
		jobjectArray j_sample_list = env->NewObjectArray(sample_list.size(), c, env->NewObject(c, constructor, env->NewStringUTF(""), env->NewStringUTF(""), env->NewStringUTF(""), env->NewStringUTF("")));

		for (int sample_index = 0; sample_index < sample_list.size(); sample_index++)
		{
			const SampleInfo &sample_info = sample_list[sample_index];

			jstring id       = env->NewStringUTF(sample_info.id.c_str());
			jstring category = env->NewStringUTF(sample_info.category.c_str());
			jstring name     = env->NewStringUTF(sample_info.name.c_str());
			jstring desc     = env->NewStringUTF(sample_info.description.c_str());

			env->SetObjectArrayElement(j_sample_list, sample_index, env->NewObject(c, constructor, id, category, name, desc));
		}

		return j_sample_list;
	}
}
#endif        // VK_USE_PLATFORM_ANDROID_KHR

namespace vkb
{
namespace
{
inline void print_info()
{
	std::string col_delim(30, '-');

	LOGI("Vulkan Best Practice Samples");
	LOGI("");
	LOGI("\tA collection of samples to demonstrate the Vulkan best practice for mobile developers.");
	LOGI("");
	LOGI("Available samples:");
	LOGI("");
	LOGI("{:20s} | {:20s} | {:20s}", "Id", "Name", "Description");
	LOGI("{}---{}---{}", col_delim.c_str(), col_delim.c_str(), col_delim.c_str());

	for (auto &sample_info : sample_list)
	{
		LOGI("{:20s} | {:20s} | {}", sample_info.id.c_str(), sample_info.name.c_str(), sample_info.description.c_str());
	}

	LOGI("");
	LOGI("Project home: https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers");
	LOGI("");
}

inline std::vector<SampleInfo>::const_iterator get_sample_info(const std::string &sample_id)
{
	return std::find_if(sample_list.begin(), sample_list.end(),
	                    [&sample_id](const SampleInfo &sample) { return sample.id == sample_id; });
}

inline const CreateAppFunc &get_create_func(const std::string &id)
{
	// Try to find the sample entry point
	const auto sample_iter = sample_create_functions.find(id);

	if (sample_iter == std::end(sample_create_functions))
	{
		// If not found, try to find the test entry point
		const auto test_iter = test_create_functions.find(id);

		if (test_iter == std::end(test_create_functions))
		{
			throw std::runtime_error("Failed to find a create function for " + id);
		}

		return test_iter->second;
	}

	return sample_iter->second;
}
}        // namespace

VulkanBestPractice::VulkanBestPractice()
{
	set_usage(
	    R"(Vulkan Best Practice.
	Usage:
		vulkan_best_practice <sample>
		vulkan_best_practice (--sample <arg> | --test <arg> | --batch <arg>) [--benchmark <frames>] [--width <arg>] [--height <arg>] [--headless] 
		vulkan_best_practice --help

	Options:
		--help                    Show this screen.
		--sample SAMPLE_ID        Run a sample.
		--test TEST_ID            Run a test.
		--batch CATEGORY_NAME     Run all samples within a category, specify 'all' to run all.
		--benchmark FRAMES        Run a fixed time step of a sample for n frames.
		--width WIDTH             The width of the screen if visible [default: 1280].
		--height HEIGHT           The height of the screen if visible [default: 720].
		--headless                Renders directly to display, skipping window creation.
	)");
}

bool VulkanBestPractice::prepare(Platform &platform)
{
	this->platform = &platform;

	if (options.contains("--help"))
	{
		print_info();
		options.print_usage();
		return false;
	}

	auto result = false;

	if (options.contains("--batch"))
	{
		auto &category_arg = options.get_string("--batch");

		if (category_arg != "all")
		{
			std::copy_if(sample_list.begin(), sample_list.end(), std::back_inserter(batch_mode_sample_list),
			             [category_arg](const SampleInfo &sample) { return sample.category == category_arg; });
		}
		else
		{
			batch_mode_sample_list = sample_list;
		}

		if (batch_mode_sample_list.empty())
		{
			LOGE("Couldn't find any samples by the given category");
			return false;
		}

		this->batch_mode_sample_iter = batch_mode_sample_list.begin();

		result = prepare_active_app(
		    sample_create_functions.at(batch_mode_sample_list.begin()->id),
		    batch_mode_sample_list.begin()->name,
		    false,
		    true);
	}
	else if (options.contains("--sample"))
	{
		const auto &sample_arg = options.get_string("--sample");

		result = prepare_active_app(
		    get_create_func(sample_arg),
		    get_sample_info(sample_arg)->name,
		    false,
		    false);
	}
	else if (options.contains("<sample>"))
	{
		const auto &sample_arg = options.get_string("<sample>");

		result = prepare_active_app(
		    get_create_func(sample_arg),
		    get_sample_info(sample_arg)->name,
		    false,
		    false);
	}
	else if (options.contains("--test"))
	{
		const auto &test_arg = options.get_string("--test");

		result = prepare_active_app(
		    get_create_func(test_arg),
		    test_arg,
		    true,
		    false);
	}
	else
	{
		// The user didn't supply any arguments so print the usage
		print_info();
		options.print_usage();
		LOGE("No arguments given, exiting");
		return false;
	}

	if (!result)
	{
		LOGE("Failed to prepare application");
	}

	return result;
}

bool VulkanBestPractice::prepare_active_app(CreateAppFunc create_app_func, const std::string &name, bool test, bool batch)
{
	if (active_app)
	{
		active_app->finish();
	}

	active_app.reset();
	active_app = create_app_func();
	active_app->set_name(name);

	skipped_first_frame = false;

	if (!active_app)
	{
		LOGE("Failed to create a valid vulkan app.");
		return false;
	}

	if (!test)
	{
		auto *app = active_app.get();
		if (auto *active_app = dynamic_cast<vkb::VulkanSample *>(app))
		{
			active_app->get_configuration().reset();
		}
	}

	if (batch)
	{
		this->batch_mode = true;
	}
	else if (is_benchmark_mode())
	{
		active_app->set_benchmark_mode(true);
	}

	active_app->set_headless(is_headless());

	auto result = active_app->prepare(*platform);

	if (!result)
	{
		LOGE("Failed to prepare vulkan app.");
		return result;
	}

	return result;
}

void VulkanBestPractice::update(float delta_time)
{
	if (active_app)
	{
		active_app->step();
	}

	elapsed_time += skipped_first_frame ? delta_time : 0.0f;
	skipped_first_frame = true;

	if (batch_mode)
	{
		// When the runtime for the current configuration is reached, advance to the next config or next sample
		if (elapsed_time >= sample_run_time_per_configuration)
		{
			elapsed_time = 0.0f;

			// Only check and advance the config if the application is a vulkan sample
			if (auto *vulkan_app = dynamic_cast<vkb::VulkanSample *>(active_app.get()))
			{
				auto &configuration = vulkan_app->get_configuration();

				if (configuration.next())
				{
					configuration.set();
					return;
				}
			}

			// Wrap it around to the start
			++batch_mode_sample_iter;
			if (batch_mode_sample_iter == batch_mode_sample_list.end())
			{
				batch_mode_sample_iter = batch_mode_sample_list.begin();
			}

			// Prepare next application
			auto result = prepare_active_app(
			    sample_create_functions.at(batch_mode_sample_iter->id),
			    batch_mode_sample_iter->name,
			    false,
			    true);

			if (!result)
			{
				LOGE("Failed to prepare vulkan sample.");
				platform->close();
			}
		}
	}
}

void VulkanBestPractice::finish()
{
	if (active_app)
	{
		active_app->finish();
	}
}

void VulkanBestPractice::resize(const uint32_t width, const uint32_t height)
{
	if (active_app)
	{
		active_app->resize(width, height);
	}
}

void VulkanBestPractice::input_event(const InputEvent &input_event)
{
	if (active_app && !batch_mode && !is_benchmark_mode())
	{
		active_app->input_event(input_event);
	}
	else
	{
		Application::input_event(input_event);
	}
}

}        // namespace vkb

std::unique_ptr<vkb::Application> create_vulkan_best_practice()
{
	return std::make_unique<vkb::VulkanBestPractice>();
}
