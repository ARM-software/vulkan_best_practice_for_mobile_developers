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

cmake_minimum_required(VERSION 3.10)

set(SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(ROOT_DIR ${SCRIPT_DIR}/../..)

set(CMAKE_FILE ${SCRIPT_DIR}/template/sample/CMakeLists.txt.in)
set(SAMPLE_SOURCE_FILE ${SCRIPT_DIR}/template/sample/sample.cpp.in)
set(SAMPLE_HEADER_FILE ${SCRIPT_DIR}/template/sample/sample.h.in)

set(SAMPLE_NAME "" CACHE STRING "")
set(OUTPUT_DIR "${ROOT_DIR}/samples" CACHE PATH "")

function(convert_file_name)
    set(options)
    set(oneValueArgs INPUT OUTPUT)
    set(multiValueArgs)
    
    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # insert an underscore before any upper case letter
    string(REGEX REPLACE "(.)([A-Z][a-z]+)" "\\1_\\2" result ${TARGET_INPUT})
    # insert an underscore before any upper case letter
    string(REGEX REPLACE "([a-z0-9])([A-Z])" "\\1_\\2" result ${result})   
    # transform characters to lower case
    string(TOLOWER ${result} result)
    
    set(${TARGET_OUTPUT} ${result} PARENT_SCOPE)
endfunction()

if(NOT SAMPLE_NAME)
    message(FATAL_ERROR "Sample name cannot be empty.")
endif()

convert_file_name(
    INPUT ${SAMPLE_NAME} 
    OUTPUT SAMPLE_NAME_FILE)

configure_file(${CMAKE_FILE} ${OUTPUT_DIR}/${SAMPLE_NAME_FILE}/CMakeLists.txt @ONLY)
configure_file(${SAMPLE_SOURCE_FILE} ${OUTPUT_DIR}/${SAMPLE_NAME_FILE}/${SAMPLE_NAME_FILE}.cpp)
configure_file(${SAMPLE_HEADER_FILE} ${OUTPUT_DIR}/${SAMPLE_NAME_FILE}/${SAMPLE_NAME_FILE}.h)
