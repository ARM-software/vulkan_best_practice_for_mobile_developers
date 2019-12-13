<!--
- Copyright (c) 2019, Arm Limited and Contributors
-
- SPDX-License-Identifier: MIT
-
- Permission is hereby granted, free of charge,
- to any person obtaining a copy of this software and associated documentation files (the "Software"),
- to deal in the Software without restriction, including without limitation the rights to
- use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
- and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
-
- The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
-
- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
- IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
- WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-
-->

# Vulkan Best Practice for Mobile Developers <!-- omit in toc -->

![Vulkan Best Practice for Mobile Developers banner](banner.jpg)

## Contents <!-- omit in toc -->

- [Introduction](#introduction)
  - [Goals](#goals)
- [Tutorials](#tutorials)
- [Setup](#setup)
- [Build](#build)
  - [Supported Platforms](#supported-platforms)
- [Usage](#usage)
- [Testing](#tests)
- [License](#license)
  - [Trademarks](#trademarks)
- [Contributions](#contributions)
- [Related resources](#related-resources)

## Introduction

The Vulkan Best Practice for Mobile Developers is collection of resources to help you develop optimized Vulkan applications for mobile platforms.

### Goals
- Create a collection of resources that demonstrate best-practice recommendations in Vulkan
- Create tutorials that explain the implementation of best-practices and include performance analysis guides
- Create a framework that can be used as reference material and also as a sandbox for advanced experimentation with Vulkan

> **Disclaimer:** This project covers advanced Vulkan concepts. If you are new to Vulkan here are a few links to get you started:
> - [Beginners Guide to Vulkan](https://www.khronos.org/blog/beginners-guide-to-vulkan)
> - [Get Started in Vulkan](https://vulkan-tutorial.com/)

## Tutorials
- **Project Basics**
  - [Controls](./docs/misc.md#controls)
  - [Debug window](./docs/misc.md#debug-window)
  - [Create a Sample](./docs/create_sample.md)
- **Vulkan Essentials**  
  - [How does Vulkan compare to OpenGL ES? What should you expect when targeting Vulkan?](./samples/vulkan_basics.md)
- **Vulkan Swapchains**  
  - [Appropriate use of N-buffering](./samples/performance/swapchain_images/swapchain_images_tutorial.md)
  - [Appropriate use of surface rotation](./samples/performance/surface_rotation/surface_rotation_tutorial.md)
- **Pipelines**
  - [Use of pipeline caches to avoid startup latency](./samples/performance/pipeline_cache/pipeline_cache_tutorial.md)
  - [Utilizing Specialization Constants](./samples/performance/specialization_constants/specialization_constants_tutorial.md)
- **Descriptors**
  - [Descriptor and buffer management](./samples/performance/descriptor_management/descriptor_management_tutorial.md)
- **Render Passes**
  - [Appropriate use of load/store operations, and use of transient attachments](./samples/performance/render_passes/render_passes_tutorial.md)
  - [Choosing the correct layout when transitioning images](./samples/performance/layout_transitions/layout_transitions_tutorial.md)
- **Render Subpasses**
  - [Benefits of subpasses over multiple render passes, use of transient attachments, and G-buffer recommended size](./samples/performance/render_subpasses/render_subpasses_tutorial.md)
- **Workload Synchronization**
  - [Using pipeline barriers efficiently](./samples/performance/pipeline_barriers/pipeline_barriers_tutorial.md)
  - [How to synchronize back to the CPU and avoid stalling](./samples/performance/wait_idle/wait_idle_tutorial.md)
- **Command Buffers**
  - [Allocation and management of command buffers](./samples/performance/command_buffer_usage/command_buffer_usage_tutorial.md#Recycling-strategies)
  - [Multi-threaded recording with secondary command buffers](./samples/performance/command_buffer_usage/command_buffer_usage_tutorial.md#Multi-threaded-recording)
- **AFBC**
  - [Appropriate use of AFBC](./samples/performance/afbc/afbc_tutorial.md)
- **Misc**
  - [Driver version](./docs/misc.md#driver-version)
  - [Memory limits](./docs/memory_limits.md)
  - [Vulkan FAQ](./docs/faq.md)

## Setup

Clone the repo with submodules using the following command:

```
git clone --recurse-submodules https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers.git
cd vulkan_best_practice_for_mobile_developers
```

Follow build instructions for your platform below.

## Build

### Supported Platforms
- Windows - [Build Guide](./docs/build.md#windows "Windows Build Guide")
- Linux - [Build Guide](./docs/build.md#linux "Linux Build Guide")
- macOS - [Build Guide](./docs/build.md#macos "macOS Build Guide")
- Android - [Build Guide](./docs/build.md#android "Android Build Guide")

## Usage

The following shows some example command line usage on how to configure and run the Vulkan Best Practices.

```
# Run Swapchain Images sample
vulkan_best_practice swapchain_images

# Run AFBC sample in benchmark mode for 5000 frames
vulkan_best_practice --sample afbc --benchmark 5000

# Run bonza test offscreen
vulkan_best_practice --test bonza --hide

# Run all the performance samples
vulkan_best_practice --batch performance
```


## Tests

- System Test - [Usage Guide](docs/testing.md#system-test "System Test Guide")
- Generate Sample - [Usage Guide](docs/testing.md#generate-sample-test "Generate Sample Test Guide")


## License

See [LICENSE](LICENSE).

This project has some third-party dependencies, each of which may have independent licensing:

- [astc-encoder](https://github.com/ARM-software/astc-encoder): ASTC Evaluation Codec
- [CTPL](https://github.com/vit-vit/CTPL): Thread Pool Library
- [docopt](https://github.com/docopt/docopt.cpp): A C++11 port of the Python argument parsing library
- [glfw](https://github.com/glfw/glfw): A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input
- [glm](https://github.com/g-truc/glm): OpenGL Mathematics
- [glslang](https://github.com/KhronosGroup/glslang): Shader front end and validator
- [dear imgui](https://github.com/ocornut/imgui): Immediate Mode Graphical User Interface
  - [dear imgui shaders](https://github.com/SaschaWillems/Vulkan/tree/master/data/shaders/imgui): GLSL shaders for dear imgui
- [HWCPipe](https://github.com/ARM-software/HWCPipe): Interface to mobile Hardware Counters
- [KTX-Software](https://github.com/KhronosGroup/KTX-Software): Khronos Texture Library and Tools
- [spdlog](https://github.com/gabime/spdlog): Fast C++ logging library
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross): Parses and converts SPIR-V to other shader languages
- [stb](https://github.com/nothings/stb): Single-file public domain (or MIT licensed) libraries
- [tinygltf](https://github.com/syoyo/tinygltf): Header only C++11 tiny glTF 2.0 library
  - [nlohmann json](https://github.com/nlohmann/json): C++ JSON Library (included by [tinygltf](https://github.com/syoyo/tinygltf))
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): Vulkan Memory Allocator
- [volk](https://github.com/zeux/volk): Meta loader for Vulkan API
- [vulkan](https://github.com/KhronosGroup/Vulkan-Docs): Sources for the formal documentation of the Vulkan API

This project uses the following 3D models. Each one has its own licence.

- Sponza: [CC BY 3.0 license](https://creativecommons.org/licenses/by/3.0/) with the following modifications:
   - All textures are converted to ASTC in .ktx format.
   - Converted to gltf using [Blender exporter](https://github.com/KhronosGroup/glTF-Blender-IO).
- Bonza: [MIT license](LICENSE)
- Space Module: [MIT license](LICENSE)

Sponza model downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data).

Fonts downloaded from [Google Fonts](https://fonts.google.com), under license [Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0)

PBR References:

- [Frostbite to PBR course notes](https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf)
- [Learn OpenGL](https://learnopengl.com/PBR/Theory)


### Trademarks

Vulkan is a registered trademark of the Khronos Group Inc.

## Contributions

All contributions are accepted under the same [LICENSE](LICENSE).

## Related resources

- [Mali GPU Best Practices](https://developer.arm.com/solutions/graphics/developer-guides/mali-gpu-best-practices): A document with recommendations for efficient API usage
- [PerfDoc](https://github.com/ARM-software/perfdoc): A Vulkan layer which aims to validate applications against Mali GPU Best Practices
