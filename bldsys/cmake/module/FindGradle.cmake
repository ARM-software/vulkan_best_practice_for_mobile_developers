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
    set(GRADLE_FILE "gradle.bat")
else()
    set(GRADLE_FILE "gradle")
endif()

find_program(GRADLE_EXECUTABLE
    NAMES 
        ${GRADLE_FILE}
    PATH_SUFFIXES 
        bin
    PATHS
        $ENV{GRADLE_HOME})

mark_as_advanced(GRADLE_EXECUTABLE)

if(GRADLE_EXECUTABLE)
    execute_process(
        COMMAND ${GRADLE_EXECUTABLE} --version
        OUTPUT_VARIABLE GRADLE_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    
	string(REGEX MATCH  "Gradle [0-9]+.[0-9]+" GRADLE_VERSION ${GRADLE_VERSION})
    string(REPLACE "Gradle " "" GRADLE_VERSION ${GRADLE_VERSION})
endif()
        
find_package_handle_standard_args(Gradle 
    VERSION_VAR GRADLE_VERSION
    REQUIRED_VARS GRADLE_EXECUTABLE)
