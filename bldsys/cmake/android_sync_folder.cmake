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

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_MODULE_PATH}/module)
set(FOLDER_DIR ${FOLDER_DIR})
set(DEVICE_DIR ${DEVICE_DIR})

find_package(Adb 1.0.39 REQUIRED)

# Ensure that directory exists in the target

set(ADB_COMMAND ${ADB_EXECUTABLE} shell mkdir -p ${DEVICE_DIR})

execute_process(
     COMMAND
     ${ADB_COMMAND})

# Sync files

set(ADB_COMMAND ${ADB_EXECUTABLE} push --sync ${FOLDER_DIR} ${DEVICE_DIR})

execute_process(
     COMMAND
     ${ADB_COMMAND}
     RESULT_VARIABLE
     ret_var
     OUTPUT_VARIABLE
     ret_msg
     OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT "${ret_var}" STREQUAL "0")
    message(WARNING "Could not sync ${FOLDER_DIR} to device:\n${ret_msg}")
else()
    message(STATUS "Updating ${FOLDER_DIR}:\n${ret_msg}")
endif()
