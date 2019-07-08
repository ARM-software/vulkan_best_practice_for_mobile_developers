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

#include "sample_controller.h"

#include "common/logging.h"
#include "platform/platform.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include <jni.h>

std::string java_argument_string = "";

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

	JNIEXPORT void JNICALL
	    Java_com_arm_vulkan_1best_1practice_BPSampleActivity_setArgumentString(JNIEnv *env, jobject thiz, jstring argument_string)
	{
		const char *arg = env->GetStringUTFChars(argument_string, 0);

		java_argument_string = std::string(arg);

		env->ReleaseStringUTFChars(argument_string, arg);
	}
}
#endif        // VK_USE_PLATFORM_ANDROID_KHR

namespace vkb
{
namespace
{
inline void print_usage()
{
	std::string col_delim(30, '-');

	LOGI("Vulkan Best Practice Samples");
	LOGI("");
	LOGI("\tA collection of samples to demonstrate the Vulkan best practice for mobile developers.");
	LOGI("");
	LOGI("Options:");
	LOGI("");
	LOGI("\t help - Display this usage guide.");
	LOGI("\t <SAMPLE_ID> - Run the specific sample.");
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

inline std::vector<SampleInfo>::const_iterator find_sample(const std::string &sample_id)
{
	return std::find_if(sample_list.begin(), sample_list.end(),
	                    [&sample_id](const SampleInfo &sample) { return sample.id == sample_id; });
}

inline std::vector<SampleInfo> find_samples_by_category(const std::string &category)
{
	std::vector<SampleInfo> runnable_sample_list;
	std::copy_if(sample_list.begin(), sample_list.end(), std::back_inserter(runnable_sample_list),
	             [category](const SampleInfo &sample) { return sample.category == category; });
	return runnable_sample_list;
}

inline const CreateAppFunc &get_sample_create_func(const std::string &sample_id)
{
	// Find the function to create sample
	const auto sample_iter = sample_create_functions.find(sample_id);

	if (sample_iter == std::end(sample_create_functions))
	{
		throw std::runtime_error("Failed to find create function for sample.");
	}

	return sample_iter->second;
}
}        // namespace

bool SampleController::prepare(Platform &platform)
{
	this->platform       = &platform;
	this->samples_to_run = sample_list;

	bool result{true};

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	platform.parse_arguments(java_argument_string);
#endif

	auto &args = platform.get_arguments();

	if (args.contains("test"))
	{
		run_test(args.at("test"));
		return false;
	}

	result = process_arguments(args);

	if (!result)
	{
		return false;
	}

	if (std::find_if(std::begin(samples_to_run), std::end(samples_to_run), [this](auto &sample) { return sample.id == current_sample->id; }) == std::end(samples_to_run))
	{
		LOGE("No sample name defined to run.");

		return false;
	}

	if (!prepare_sample(current_sample))
	{
		LOGE("Failed to prepare sample");
		return false;
	}

	return true;
}

bool SampleController::prepare_sample(std::vector<SampleInfo>::const_iterator sample)
{
	active_app.reset();

	auto &create_sample_func = get_sample_create_func(sample->id);

	active_app = create_sample_func();
	active_app->set_name(sample->name);

	if (!active_app)
	{
		LOGE("Failed to create a valid vulkan sample.");

		return false;
	}

	if (!active_app->prepare(*platform))
	{
		LOGE("Failed to prepare vulkan sample.");

		return false;
	}

	active_app->get_configuration().reset();

	return true;
}

bool SampleController::process_arguments(const ArgumentParser &args)
{
	if (args.contains("help"))
	{
		print_usage();
		return false;
	}

	if (args.contains("category"))
	{
		auto &category_arg = args.at("category");
		if (std::find(category_list.begin(), category_list.end(), category_arg) != category_list.end())
		{
			samples_to_run = find_samples_by_category(category_arg);
			automate       = true;
			current_sample = samples_to_run.begin();
		}
	}
	else if (args.contains("sample"))
	{
		automate       = false;
		current_sample = find_sample(args.at("sample"));
	}
	else
	{
		automate       = true;
		current_sample = samples_to_run.begin();
	}

	return true;
}

bool SampleController::run_test(const std::string &test_name)
{
	// Run a test app
	automate               = false;
	auto &create_test_func = test_create_functions.at(test_name);

	active_app = create_test_func();
	active_app->set_name(test_name);

	if (!active_app)
	{
		LOGE("Failed to create a test.");

		return false;
	}

	if (!active_app->prepare(*platform))
	{
		LOGE("Failed to prepare vulkan test.");

		return false;
	}

	active_app->step();

	return true;
}

void SampleController::update(float delta_time)
{
	if (active_app)
	{
		active_app->step();
	}

	elapsed_time += skipped_first_frame ? delta_time : 0.0f;
	skipped_first_frame = true;

	if (automate)
	{
		auto &configuration = active_app->get_configuration();

		if (elapsed_time >= sample_run_time_per_configuration)
		{
			elapsed_time = 0.0f;

			if (!configuration.next())
			{
				++current_sample;
				if (current_sample == samples_to_run.end())
				{
					current_sample = samples_to_run.begin();
				}

				skipped_first_frame = false;
				if (!prepare_sample(current_sample))
				{
					LOGE("Failed to prepare vulkan sample.");
					platform->close();
				}
			}
			else
			{
				configuration.set();
			}
		}
	}
}

void SampleController::finish()
{
	if (active_app)
	{
		active_app->finish();
	}
}

void SampleController::resize(const uint32_t width, const uint32_t height)
{
	if (active_app)
	{
		active_app->resize(width, height);
	}
}

void SampleController::input_event(const InputEvent &input_event)
{
	if (active_app && !automate)
	{
		active_app->input_event(input_event);
	}
	else
	{
		Application::input_event(input_event);
	}
}

}        // namespace vkb

std::unique_ptr<vkb::Application> create_sample_controller()
{
	return std::make_unique<vkb::SampleController>();
}
