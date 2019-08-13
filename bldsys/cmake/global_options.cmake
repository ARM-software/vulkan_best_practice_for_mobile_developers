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

if(NOT CMAKE_ANDROID_NDK)
    set(CMAKE_ANDROID_NDK ${ANDROID_NDK})
endif()

# Enable group projects in folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

if(ANDROID)
    set(TARGET_ARCH ${CMAKE_ANDROID_ARCH_ABI})
else()
    set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

set(VKB_WARNINGS_AS_ERRORS ON CACHE BOOL "Enable Warnings as Errors")
set(VKB_ENTRYPOINTS OFF CACHE BOOL "Enable create entrypoint project for every application.")
set(VKB_SYMLINKS OFF CACHE BOOL "Enable create symlink folders for every application.")
set(VKB_VALIDATION_LAYERS OFF CACHE BOOL "Enable validation layers for every application.")
set(VKB_BUILD_SAMPLES ON CACHE BOOL "Enable generation and building of Vulkan best practice samples.")
set(VKB_BUILD_TESTS OFF CACHE BOOL "Enable generation and building of Vulkan best practice tests.")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/${CMAKE_BUILD_TYPE}/${TARGET_ARCH}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

string(LENGTH "${CMAKE_SOURCE_DIR}/" ROOT_PATH_SIZE)
add_definitions(-DROOT_PATH_SIZE=${ROOT_PATH_SIZE})

set(CMAKE_C_FLAGS_DEBUG   "-DDEBUG=0 ${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG=0 ${CMAKE_CXX_FLAGS_DEBUG}")