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

function(scan_dirs)
    set(options)
    set(oneValueArgs LIST DIR)
    set(multiValueArgs)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EXISTS ${TARGET_DIR})
        message(FATAL_ERROR "Directory not found `${TARGET_DIR}`")
    endif()

    file(GLOB DIR_FILES RELATIVE ${TARGET_DIR} ${TARGET_DIR}/*)

    set(DIR_LIST)

    foreach(FILE_NAME ${DIR_FILES})
        if(IS_DIRECTORY ${TARGET_DIR}/${FILE_NAME})
            list(APPEND DIR_LIST ${FILE_NAME})
        endif()
    endforeach()

    set(${TARGET_LIST} ${DIR_LIST} PARENT_SCOPE)
endfunction()

function(create_symlink)
    set(options)
    set(oneValueArgs NAME DIR LINK WORK_DIR)
    set(multiValueArgs)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(WIN32)
        file(TO_NATIVE_PATH ${TARGET_LINK} DIR_LINK)
        file(TO_NATIVE_PATH ${TARGET_DIR} DIR_SRC)

        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMENT "Create symlink for ${TARGET_DIR}"
            COMMAND if exist ${DIR_LINK} rmdir /q ${DIR_LINK}
            COMMAND mklink /D ${DIR_LINK} ${DIR_SRC}
            VERBATIM)
    else()
        add_custom_command(
            TARGET ${TARGET_NAME} PRE_BUILD
            COMMENT "Create symlink for ${TARGET_DIR}"
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${TARGET_DIR} ${TARGET_LINK}
            VERBATIM)
    endif()
endfunction()

function(string_join)
    set(options)
    set(oneValueArgs GLUE)
    set(multiValueArgs INPUT OUTPUT)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(REPLACE ";" "${TARGET_GLUE}" RESULT_STR "${TARGET_INPUT}")

    set(${TARGET_OUTPUT} ${RESULT_STR} PARENT_SCOPE)
endfunction()
