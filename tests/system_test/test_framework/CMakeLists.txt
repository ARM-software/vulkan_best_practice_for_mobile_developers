# Copyright (c) 2019, Arm Limited and Contributors
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge,
# to any person obtaining a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

cmake_minimum_required(VERSION 3.8)

project(test_framework LANGUAGES C CXX)

set(FRAMEWORK_FILES 
    # Header files
    gltf_loader_test.h
    vulkan_test.h 
    # Source Files
    gltf_loader_test.cpp
    vulkan_test.cpp)

source_group("\\" FILES ${FRAMEWORK_FILES})

add_library(${PROJECT_NAME} STATIC ${FRAMEWORK_FILES})

# inherit compile definitions from framework target
target_compile_definitions(${PROJECT_NAME} PUBLIC 
    $<TARGET_PROPERTY:framework,COMPILE_DEFINITIONS>)

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<TARGET_PROPERTY:framework,INCLUDE_DIRECTORIES>
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
