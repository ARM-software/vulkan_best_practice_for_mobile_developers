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

#include "platform/file.h"

#include <jni.h>

namespace vkb::file
{
std::string android_asset_path;
std::string android_temp_path;
std::string android_storage_path;

extern "C"
{
	JNIEXPORT void JNICALL
	    Java_com_arm_vulkan_1best_1practice_BPSampleActivity_initFilePath(JNIEnv *env, jobject thiz, jstring asset_path, jstring temp_path, jstring storage_path)
	{
		const char *asset_path_cstr = env->GetStringUTFChars(asset_path, 0);
		android_asset_path          = std::string(asset_path_cstr) + "/";
		env->ReleaseStringUTFChars(asset_path, asset_path_cstr);

		const char *temp_path_cstr = env->GetStringUTFChars(temp_path, 0);
		android_temp_path          = std::string(temp_path_cstr) + "/";
		env->ReleaseStringUTFChars(temp_path, temp_path_cstr);

		const char *storage_path_cstr = env->GetStringUTFChars(storage_path, 0);
		android_storage_path          = std::string(storage_path_cstr) + "/";
		env->ReleaseStringUTFChars(storage_path, storage_path_cstr);
	}
}

std::string Path::get_asset_path()
{
	return android_asset_path;
}

std::string Path::get_temp_path()
{
	return android_temp_path;
}

std::string Path::get_storage_path()
{
	return android_storage_path;
}
}        // namespace vkb::file
