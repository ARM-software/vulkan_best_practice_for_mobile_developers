#[[
 Copyright (c) 2019, Arm Limited and Contributors

 SPDX-License-Identifier: MIT

 Permission is hereby granted, free of charge,
 to any person obtaining a copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ]]

# Android Nsight Tegra version requirement
set(REQUIRED_NSIGHT_TEGRA_VERSION "3.4")

# Get Android Nsight Tegra environment
set(NSIGHT_TEGRA_VERSION ${CMAKE_VS_NsightTegra_VERSION})
if( "${NSIGHT_TEGRA_VERSION}" STREQUAL "" )
	get_filename_component(NSIGHT_TEGRA_VERSION "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\NVIDIA Corporation\\Nsight Tegra;Version]" NAME)
endif()

# Report and check version if it exist
if(NOT "${NSIGHT_TEGRA_VERSION}" STREQUAL "")
	message(STATUS "Android Nsight Tegra version found: ${NSIGHT_TEGRA_VERSION}")
	if(NOT "${NSIGHT_TEGRA_VERSION}" VERSION_GREATER_EQUAL "${REQUIRED_NSIGHT_TEGRA_VERSION}+")
		message(FATAL_ERROR "Expected Android Nsight Tegra version: ${REQUIRED_NSIGHT_TEGRA_VERSION}")
	endif()
endif()

# We are building Android platform, fail if Android Nsight Tegra not found
if( NOT NSIGHT_TEGRA_VERSION )
	message(FATAL_ERROR "Engine requires Android Nsight Tegra to be installed in order to build Android platform.")
endif()

file(TO_CMAKE_PATH $ENV{ANDROID_NDK_ROOT} CMAKE_ANDROID_NDK)
if(NOT CMAKE_ANDROID_NDK)
	message(FATAL_ERROR "Engine requires CMAKE_ANDROID_NDK environment variable to point to your Android NDK.")
endif()

# Tell CMake we are cross-compiling to Android
set(CMAKE_SYSTEM_NAME Android)

# Tell CMake which version of the Android API we are targeting
set(CMAKE_ANDROID_API_MIN 24 CACHE STRING "")
set(CMAKE_ANDROID_API 24 CACHE STRING "")

set(CMAKE_ANDROID_ARCH "arm64-v8a" CACHE STRING "")

# Tell CMake we don't want to skip Ant step
set(CMAKE_ANDROID_SKIP_ANT_STEP 0)

# Use Clang as the C/C++ compiler
set(CMAKE_GENERATOR_TOOLSET DefaultClang)

# Tell CMake we have our java source in the 'java' directory
set(CMAKE_ANDROID_JAVA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/bldsys/android/vulkan_best_practice/src/main/java)

set(CMAKE_ANDROID_ASSETS_DIRECTORIES ${CMAKE_SOURCE_DIR}/assets)

# Tell CMake we have use Gradle as our default build system
set(CMAKE_ANDROID_BUILD_SYSTEM GradleBuild)