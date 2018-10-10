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

set(CMAKE_SYSTEM_NAME Android)

set(ANDROID_NDK_ROOT CACHE STRING "")

if(DEFINED ENV{ANDROID_NDK_ROOT} AND NOT ANDROID_NDK_ROOT)
    file(TO_CMAKE_PATH $ENV{ANDROID_NDK_ROOT} ANDROID_NDK_ROOT)
    set(ANDROID_NDK_ROOT ${ANDROID_NDK_ROOT} CACHE STRING "" FORCE)
else()
    set(ANDROID_STANDALONE_TOOLCHAIN CACHE STRING "")

    if(DEFINED ENV{ANDROID_STANDALONE_TOOLCHAIN})
        file(TO_CMAKE_PATH $ENV{ANDROID_STANDALONE_TOOLCHAIN} ANDROID_STANDALONE_TOOLCHAIN)
        set(ANDROID_STANDALONE_TOOLCHAIN ${ANDROID_STANDALONE_TOOLCHAIN} CACHE STRING "" FORCE)
    endif()
endif()

if(NOT ANDROID_NDK_ROOT AND NOT ANDROID_STANDALONE_TOOLCHAIN)
	message(FATAL_ERROR "Required ANDROID_NDK_ROOT or ANDROID_STANDALONE_TOOLCHAIN environment variable to point to your Android NDK Root/Android Standalone Toolchain.")
endif()

set(CMAKE_ANDROID_NDK ${ANDROID_NDK_ROOT})
set(CMAKE_ANDROID_STANDALONE_TOOLCHAIN ${ANDROID_STANDALONE_TOOLCHAIN})

set(CMAKE_ANDROID_API 24 CACHE STRING "")

set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a" CACHE STRING "")

set(CMAKE_ANDROID_STL_TYPE "c++_static" CACHE STRING "")

set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION "clang" CACHE STRING "")
