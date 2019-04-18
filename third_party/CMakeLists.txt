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

project(third_party LANGUAGES C CXX)

# glm
add_library(glm INTERFACE)

target_include_directories(glm INTERFACE "glm")

target_compile_definitions(glm INTERFACE 
    GLM_FORCE_SWIZZLE 
    GLM_FORCE_RADIANS 
    GLM_FORCE_CTOR_INIT 
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_CXX14)

# tinygltf
add_library(tinygltf INTERFACE)

target_include_directories(tinygltf INTERFACE "tinygltf")

# vulkan
add_library(vulkan INTERFACE)

target_include_directories(vulkan INTERFACE "vulkan/include")

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

if(ANDROID)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
endif()

add_library(vma INTERFACE)

target_include_directories(vma INTERFACE "vma/src")

target_link_libraries(vma INTERFACE vulkan)

# volk
set(VOLK_FILES 
    "volk/volk.c" 
    "volk/volk.h")

add_library(volk STATIC ${VOLK_FILES})

target_link_libraries(volk PUBLIC vulkan)

target_include_directories(volk PUBLIC "volk")

set_property(TARGET volk PROPERTY FOLDER "ThirdParty")

# imgui
set(IMGUI_FILES 
    "imgui/imgui.cpp" 
    "imgui/imgui_demo.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_widgets.cpp"
    "imgui/imconfig.h"
    "imgui/imgui.h"
    "imgui/imgui_internal.h"
    "imgui/imstb_rectpack.h"
    "imgui/imstb_textedit.h"
    "imgui/imstb_truetype.h")

add_library(imgui STATIC ${IMGUI_FILES})

target_include_directories(imgui PUBLIC "imgui")

set_property(TARGET imgui PROPERTY FOLDER "ThirdParty")

# glslang
option(ENABLE_SPVREMAPPER OFF)
option(ENABLE_AMD_EXTENSIONS OFF)
option(ENABLE_GLSLANG_BINARIES OFF)
option(ENABLE_NV_EXTENSIONS OFF)
option(ENABLE_HLSL OFF)
option(ENABLE_OPT OFF)
option(BUILD_TESTING OFF)

add_subdirectory(glslang)

if (NOT TARGET glslang-default-resource-limits)
    add_library(glslang-default-resource-limits
                glslang/StandAlone/ResourceLimits.cpp)

    set_property(TARGET glslang-default-resource-limits PROPERTY FOLDER "ThirdParty")
    
    target_include_directories(glslang-default-resource-limits
                                PUBLIC glslang/StandAlone)
endif()

if(NOT MSVC)
    target_compile_options(glslang PRIVATE 
        "-Wno-logical-op-parentheses" 
        "-Wno-unused-parameter")
        
    target_compile_options(SPIRV PRIVATE 
        "-Wno-logical-op-parentheses" 
        "-Wno-unused-parameter")
endif()

set_property(TARGET glslang PROPERTY FOLDER "ThirdParty")
set_property(TARGET OGLCompiler PROPERTY FOLDER "ThirdParty")
set_property(TARGET OSDependent PROPERTY FOLDER "ThirdParty")
set_property(TARGET SPIRV PROPERTY FOLDER "ThirdParty")

# spirv-cross
add_subdirectory(spirv-cross)

set_property(TARGET spirv-cross PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-core PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-glsl PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-cpp PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-hlsl PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-msl PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-reflect PROPERTY FOLDER "ThirdParty")
set_property(TARGET spirv-cross-util PROPERTY FOLDER "ThirdParty")

# hwcpipe
add_subdirectory(hwcpipe)

set_property(TARGET hwcpipe PROPERTY FOLDER "ThirdParty")

if(ANDROID)
    # native_app_glue
    set(NATIVE_APP_GLUE_DIR "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")

    file(GLOB NATIVE_APP_GLUE_FILES "${NATIVE_APP_GLUE_DIR}/*.c" "${NATIVE_APP_GLUE_DIR}/*.h")
    
    add_library(native_app_glue STATIC ${NATIVE_APP_GLUE_FILES})
        
    target_include_directories(native_app_glue PUBLIC ${NATIVE_APP_GLUE_DIR})
    
    set_property(TARGET native_app_glue PROPERTY FOLDER "ThirdParty")
else() 
    # GLFW
    option(GLFW_BUILD_DOCS OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_BUILD_EXAMPLES OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_INSTALL OFF)
    
    add_subdirectory(glfw)
    
    set_property(TARGET glfw PROPERTY FOLDER "ThirdParty")
endif()

add_subdirectory(spdlog)
