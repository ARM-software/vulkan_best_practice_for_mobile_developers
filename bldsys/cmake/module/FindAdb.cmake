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

include(FindPackageHandleStandardArgs)

if(CMAKE_HOST_WIN32)
    set(ADB_FILE "adb.exe")
else()
    set(ADB_FILE "adb")
endif()

find_program(ADB_EXECUTABLE
    NAMES
        ${ADB_FILE}
    PATH_SUFFIXES
        bin
    PATHS
        $ENV{ANDROID_HOME})

mark_as_advanced(ADB_EXECUTABLE)

if(ADB_EXECUTABLE)
    execute_process(
        COMMAND ${ADB_EXECUTABLE} --version
        OUTPUT_VARIABLE ADB_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(ADB_VERSION_PREFIX "Android Debug Bridge version ")
	string(REGEX MATCH  "${ADB_VERSION_PREFIX}[0-9]+.[0-9]+.[0-9]+" ADB_VERSION ${ADB_VERSION})
    string(REPLACE ${ADB_VERSION_PREFIX} "" ADB_VERSION ${ADB_VERSION})
endif()

find_package_handle_standard_args(Adb
    VERSION_VAR ADB_VERSION
    REQUIRED_VARS ADB_EXECUTABLE)
