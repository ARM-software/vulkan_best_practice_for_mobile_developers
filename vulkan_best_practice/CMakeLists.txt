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

cmake_minimum_required(VERSION 3.10)

include(android_package)

# create sample app project
project(vulkan_best_practice LANGUAGES C CXX)

# generate header files
generate_samples_header(
    SAMPLE_ID_LIST ${TOTAL_SAMPLE_ID_LIST}
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

generate_tests_header(
    TEST_ID_LIST ${TOTAL_TEST_ID_LIST}
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

# generate entrypoint file (main function with per platform support)
generate_entrypoint(
    ID vulkan_best_practice
    NAME "Vulkan Best Practice Samples"
    INCLUDE_PATH vulkan_best_practice
    OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)

# create list of project files (header + source)
set(PROJECT_FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/samples.h
    ${CMAKE_CURRENT_BINARY_DIR}/tests.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_best_practice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_best_practice.cpp)
    
# add object files from samples
foreach(SAMPLE_ID ${TOTAL_SAMPLE_ID_LIST})
    if(TARGET ${SAMPLE_ID})
        get_target_property(TARGET_TYPE ${SAMPLE_ID} TYPE)

        if(TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
            list(APPEND PROJECT_FILES $<TARGET_OBJECTS:${SAMPLE_ID}>)
        endif()
    endif()
endforeach()

# add object files from tests
foreach(TEST_ID ${TOTAL_TEST_ID_LIST})
    if(TARGET ${TEST_ID})
        get_target_property(TARGET_TYPE ${TEST_ID} TYPE)

        if(TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
            list(APPEND PROJECT_FILES $<TARGET_OBJECTS:${TEST_ID}>)
        endif()
    endif()
endforeach()

source_group("\\" FILES ${PROJECT_FILES})

# select target type based on platform
if(ANDROID)
    if(CMAKE_VS_NsightTegra_VERSION)
        list(APPEND PROJECT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml)
    endif()

    add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else()
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_FILES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../samples
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/system_test/sub_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/system_test)

target_link_libraries(${PROJECT_NAME} PUBLIC framework)

if(VKB_BUILD_TESTS)
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<TARGET_PROPERTY:test_framework,INCLUDE_DIRECTORIES>)
    target_link_libraries(${PROJECT_NAME} PUBLIC test_framework)
endif()

if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-u ANativeActivity_onCreate")
    
    if(CMAKE_VS_NsightTegra_VERSION)
        set_property(TARGET ${PROJECT_NAME} PROPERTY ANDROID_GUI ON)
        set_property(TARGET ${PROJECT_NAME} PROPERTY ANDROID_ASSETS_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../assets)
        set_property(TARGET ${PROJECT_NAME} PROPERTY ANDROID_JAVA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bldsys/android/java)
    endif()
    
    # Add packaging project only if not using CMake's toolchain
    if(CMAKE_SYSTEM_VERSION GREATER 1)
        add_android_package_project(
            NAME ${PROJECT_NAME}_package
            DEPENDS ${PROJECT_NAME}
            ASSET_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../assets
            JAVA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android/java
            RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android/res
            MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml)
    endif()

    # Sync assets and shaders
    android_sync_folder(PATH ${CMAKE_CURRENT_SOURCE_DIR}/../assets)
    android_sync_folder(PATH ${CMAKE_CURRENT_SOURCE_DIR}/../shaders)
else()
    if(${VKB_SYMLINKS})
        create_symlink(
            NAME ${PROJECT_NAME}
            DIR ${CMAKE_CURRENT_SOURCE_DIR}/../assets
            LINK ${CMAKE_CURRENT_BINARY_DIR}/assets)
            
        create_symlink(
            NAME ${PROJECT_NAME}
            DIR ${CMAKE_SOURCE_DIR}/../outputs
            LINK ${CMAKE_CURRENT_BINARY_DIR}/outputs)
    endif()
endif()

if(MSVC)
    #Set the working directory to the source of the project so developer dont have to
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    #Configure output paths
    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG_TYPE} SUFFIX)
        string(TOLOWER ${CONFIG_TYPE} CONFIG_DIR)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/bin/${CONFIG_DIR}/${TARGET_ARCH})
        set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/lib/${CONFIG_DIR}/${TARGET_ARCH})
        set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/lib/${CONFIG_DIR}/${TARGET_ARCH})
    endforeach()
endif()
