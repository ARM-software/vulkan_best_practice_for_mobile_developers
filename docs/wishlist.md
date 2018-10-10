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

# What's Next?

The following wish list is a collection of common questions and issues that arise in conversations surrounding best practices when building applications using Vulkan. Our aim is to create Tutorials and Samples that cover these questions in detail.

- **Vulkan Getting Started**
  - [x] Introduction: devices, queue families, queues, swapchains, pipelines, descriptor sets, command buffers, render passes and subpasses, and commands

- **Queues**
  - [ ] More detailed look at queues, and how you might want to use them.  
  - [ ] How many queues should you have?  
  - [ ] How queues help with multi-threading?  
  - [ ] How to synchronize queues with other queues on the GPU and on the CPU?  

- **Command Buffers**
  - [ ] More detailed look at command buffers, and how you might want to use them.  
  - [ ] Primary command buffers  
  - [ ] Secondary command buffers  
  - [ ] Recycling and reuse  

- **Pipelines**
  - [ ] Benefits of compiled pipelines for optimization  
  - [ ] Beware of running out of program memory, and consider LRU shader caching   
  - [ ] Benefits of specialization constants over uniforms  
  - [ ] Use of pipeline caches to avoid startup latency  
  - [ ] Use of dynamic states and their impact on performances  

- **Descriptor Management**
  - [ ] More detailed look at descriptor pools and descriptors, and how you might want to use them.  
  - [ ] Best practices to minimize CPU load and rebuild churn (when does this advice obsolete?)  
  - [ ] Best practices for reusing and modifying descriptor sets when things have to change?  

- **Render Passes**
  - [ ] Best practices to minimize readbacks / write outs / memory footprint  
  - [ ] How to implement multisampling  
  - [ ] On-chip resolve (both color and depth/stencil using VK_KHR_depth_stencil_resolve) vs. writing out all samples and resolving using e.g. vkCmdResolveImage. Former is much lower cycles and BW overall, and additional can give you AFBC on the output.

- **Sub Passes**
  - [ ] Explain benefits of subpasses over multiple render passes  
  - [ ] Explain rules for what gets merged and what doesn't (principle of "no magic")  
  - [ ] Explain transient states for intermediate attachments  
  - [ ] Explain deferred shading best practices (first passes create ZS buffer, later passes treat it as read-only to minimize pixel dependencies).  

- **Workload Synchronization**
  - [ ] How to synchronize back to the CPU (with sufficient latency to avoid stalling)  
  - [ ] How to synchronize between GPU workloads to avoid bubbles  
  - [ ] How to synchronize compute shaders to avoid bubbles  
  
- **Gotchas**	
  - [ ] 16MB shader pipeline limit  
  - [ ] 180MB varying JIT limit  
  - [ ] Robust buffer access slowdowns  

- **Multi-threading**
  - [ ] One command buffer per pass per CPU core  
  - [ ] Clustering scene - secondary command buffer per thread  
  - [ ] Submit to same queue to run things in sequence (on a single core using external sync)  

- **Async Workloads**
  - [ ] Multiple queues to allow different work rates and overlapping   
 
- **Real Rendering Pipelines**	
  - [ ] Decals - need a depth value (depth read subpass would be cool)  
  - [ ] UI pass for FBO0 - pass command buffer from 3D pass to UI pipeline, don't spilt renderpass (subpass might also work)  
 
- **Limits**
  - [ ] Limitations on program memory  
  - [ ] Limitations on JIT memory  

- **Microscale API Usage**	
  - [ ] Many of the best practices we have relate to specific API calls, API functionality, or API options. These concepts are more like peep-hole optimizations in a compiler, so are easier to retrofit to an existing engine or code base.	

- **Microscale Resource Usage**
  - [ ] Many of the best practices we have relate to efficient data usage to minimize bandwidth and/or access cost. These concepts are often API agnostic but may encompass external tools for e.g. data format conversion or data packing.
  - [ ] Texture compression
  - [ ] Mipmapping

- **Development**
  - **Vulkan layers**
    - [ ] How to use them on Android  
    - [ ] PerfDoc
    
  - **API debug**
    - [ ] MGD  
    - [ ] RenderDoc

  - **Profiling**
    - [ ] MGD  
    - [ ] Streamline
    
  - **Shader debug**
    - [ ] Mali offline compiler  
    - [ ] SPIRV-Cross