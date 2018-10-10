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

cmake_minimum_required(VERSION 3.6)

scan_dirs(
    DIR ${CMAKE_CURRENT_SOURCE_DIR}
    LIST SAMPLE_ID_LIST)

# Add each sample from the filtered list
foreach(SAMPLE_ID ${SAMPLE_ID_LIST})
    add_subdirectory(${SAMPLE_ID})
endforeach()

# Order of the sample ids
set(ORDER_LIST 
    "hello_triangle"
    "swapchain_images"
    "surface_rotation"
    "render_passes"
    "afbc")

# Orders the sample ids by the order list above
order_sample_list(
    INPUT ${SAMPLE_ID_LIST}
    OUTPUT SAMPLE_ID_LIST
    ORDER ${ORDER_LIST})

# Make sample list visible parent scope (required by vulkan_best_practice project)
set(SAMPLE_ID_LIST ${SAMPLE_ID_LIST} PARENT_SCOPE)
