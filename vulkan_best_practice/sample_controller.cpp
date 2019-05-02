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

#include "platform/platform.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include <jni.h>

std::vector<std::string> java_arguments;

extern "C"
{
	JNIEXPORT jobjectArray JNICALL
	    Java_com_arm_vulkan_1best_1practice_BPSampleActivity_getSamples(JNIEnv *env, jobject thiz)
	{
		jclass       c             = env->FindClass("com/arm/vulkan_best_practice/Sample");
		jmethodID    constructor   = env->GetMethodID(c, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
		jobjectArray j_sample_list = env->NewObjectArray(sample_list.size(), c, env->NewObject(c, constructor, env->NewStringUTF(""), env->NewStringUTF(""), env->NewStringUTF("")));

		for (int sample_index = 0; sample_index < sample_list.size(); sample_index++)
		{
			const SampleInfo &sample_info = sample_list[sample_index];

			jstring id   = env->NewStringUTF(sample_info.id.c_str());
			jstring name = env->NewStringUTF(sample_info.name.c_str());
			jstring desc = env->NewStringUTF(sample_info.description.c_str());

			env->SetObjectArrayElement(j_sample_list, sample_index, env->NewObject(c, constructor, id, name, desc));
		}

		return j_sample_list;
	}

	JNIEXPORT void JNICALL
	    Java_com_arm_vulkan_1best_1practice_BPSampleActivity_setArguments(JNIEnv *env, jobject thiz, jobjectArray args)
	{
		int args_count = env->GetArrayLength(args);

		java_arguments.clear();

		for (int i = 0; i < args_count; i++)
		{
			jstring arg_str = (jstring)(env->GetObjectArrayElement(args, i));

			const char *arg = env->GetStringUTFChars(arg_str, 0);

			java_arguments.push_back(std::string(arg));

			env->ReleaseStringUTFChars(arg_str, arg);
		}
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
	                    [&sample_id](const SampleInfo &sample) -> bool { return sample.id == sample_id; });
}

inline const SampleCreateFunc &get_sample_create_func(const std::string &sample_id)
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
	this->platform = &platform;

	bool result{true};

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	result = parse_arguments(java_arguments);
#else
	result = parse_arguments(platform.get_arguments());
#endif

	if (!result)
	{
		return false;
	}

	if (current_sample == sample_list.end())
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
	active_sample.reset();

	auto &create_sample_func = get_sample_create_func(sample->id);

	active_sample = create_sample_func();
	active_sample->set_name(sample->name);

	if (!active_sample)
	{
		LOGE("Failed to create a valid vulkan sample.");

		return false;
	}

	if (!active_sample->prepare(*platform))
	{
		LOGE("Failed to prepare vulkan sample.");

		return false;
	}

	active_sample->get_configuration().reset();

	return true;
}

bool SampleController::parse_arguments(const std::vector<std::string> &args)
{
	if (std::find(args.begin(), args.end(), "help") != args.end())
	{
		print_usage();

		return false;
	}

	if (!args.empty())
	{
		automatic_demo_mode = false;

		current_sample = find_sample(*args.begin());
	}
	else
	{
		automatic_demo_mode = true;

		current_sample = sample_list.begin();
	}

	return true;
}

void SampleController::update(float delta_time)
{
	if (active_sample)
	{
		active_sample->step();
	}

	elapsed_time += skipped_first_frame ? delta_time : 0.0f;
	skipped_first_frame = true;

	if (automatic_demo_mode)
	{
		auto &configuration = active_sample->get_configuration();

		if (elapsed_time >= sample_run_time_per_configuration)
		{
			elapsed_time = 0.0f;

			if (!configuration.next())
			{
				++current_sample;
				if (current_sample == sample_list.end())
				{
					current_sample = sample_list.begin();
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
	if (active_sample)
	{
		active_sample->finish();
	}
}

void SampleController::resize(const uint32_t width, const uint32_t height)
{
	if (active_sample)
	{
		active_sample->resize(width, height);
	}
}

void SampleController::input_event(const InputEvent &input_event)
{
	if (active_sample && !automatic_demo_mode)
	{
		active_sample->input_event(input_event);
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
