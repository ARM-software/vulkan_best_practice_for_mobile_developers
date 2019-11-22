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

# Vulkan FAQ

- [Transitioning the swapchain image on acquisition](#transitioning-the-swapchain-image-on-acquisition)
- [Implicit image transitions via initialLayout / finalLayout](#implicit-image-transitions-via-initiallayout-finallayout)
- [Debugging a DEVICE_LOST](#debugging-a-device_lost)
- [Meaning of srcAccessMask = 0](#meaning-of-srcaccessmask-0)
- [Usage of a single image in multiple frames](#usage-of-a-single-image-in-multiple-frames)
- [Using multiple queues](#using-multiple-queues)
- [Changing a buffer's contents dynamically](#changing-a-buffers-contents-dynamically)
- [Allocating and mapping memory for a buffer](#allocating-and-mapping-memory-for-a-buffer)
- [Understanding barrier scope](#understanding-barrier-scope)
- [Number of descriptor pools](#number-of-descriptor-pools)
- [Synchronizing texture transfers](#synchronizing-texture-transfers)
- [Meaning of signaling a fence](#meaning-of-signaling-a-fence)
- [Struct alignment for uniform buffers and push constants](#struct-alignment-for-uniform-buffers-and-push-constants)
- [Crashes with no backtrace on Android](#crashes-with-no-backtrace-on-android)
- [Shader variants](#shader-variants)
- [Multithreaded rendering performance](#multithreaded-rendering-performance)


## Transitioning the swapchain image on acquisition

###### Q:

How should I transition the swapchain image to the layout I need? Is the implicit transition (`initialLayout → layout`) good enough?

###### A:

The default transition may not be enough if you want to acquire in the most efficient way possible. `VkSubmitInfo` allows to pass an acquisition semaphore with a `pWaitDstStageMask` and the optimal value is usually `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`, because only need the swapchain image to be ready when we are actually going to write to it.

The problem is that the implicit transition (`initialLayout → layout`) of the image will not wait for that stage. If there is a mismatch, the GPU might try to transition the image before it is fully acquired, with undefined results.

There are two approaches to solve this:

* Giving up on optimal acquisition, by passing `TOP_OF_PIPE` as `pWaitDstStageMask` (not recommended);
* Replace the implicit subpass dependency with an explicit one, taking the correct stage mask into account.

The acquisition semaphore's pWaitDstStageMask guarantees that the image acquisition will happen before `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`, but we don't know exactly when. Thus we need an external dependency which fixes the stage for the transition to `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`, otherwise the GPU might try to transition the image before it is acquired from the presentation engine.

This is an example subpass dependency:

```cpp
VkSubpassDependency dependency = { 0 };
dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
dependency.dstSubpass = 0;
dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
// Since we changed the image layout, we need to make the memory visible to
// color attachment to modify.
dependency.srcAccessMask = 0;
dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
```

Another approach, in case you find it simpler, is to disable the implicit dependency (i.e. set `initialLayout = layout`) and do the layout transitions via pipeline barriers. This way you can ensure that the transition happens at the correct stage, when the image has already been acquired.


## Implicit image transitions via initialLayout / finalLayout

###### Q:

I'm transitioning images between render passes using `initialLayout → layout → finalLayout` but I'm getting rendering artifacts. Why?

###### A:

This question comes out of a real life example we had when setting up a single post-processing pipeline. The screen was filled with tile-sized artifacts.

We were running a first render pass which would transition the image (`finalLayout`) to `SHADER_READ_ONLY_OPTIMAL`, then the following pass would read from it and render to the swapchain. Those rendering artifacts appeared inconsistently between devices, also depending on swapchain size, thus suggesting some sort of synchronization issue.

Vulkan requires explicit synchronization, even when it might seem that it could be inferred. The GPU can execute the render pass in any order, unless we explicitly mark the dependencies.
The problem here was that we asked Vulkan to transition the image, but we didn't really say *by when* we wanted it. In Vulkan terms:

> Automatic layout transitions into finalLayout happens-before the visibility operations for all dependencies with a dstSubpass equal to VK_SUBPASS_EXTERNAL, where srcSubpass uses the attachment that will be transitioned. For attachments created with VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT, automatic layout transitions into finalLayout happen-before the visibility operations for all dependencies with a dstSubpass equal to VK_SUBPASS_EXTERNAL, where srcSubpass uses any aliased attachment.

This is basically saying that it is not guaranteed that the next render pass will see the image in its new layout, unless we add a subpass dependency with `VK_SUBPASS_EXTERNAL`.
Subpass dependencies are baked into the render pass description, which means you'll lose flexibility in how you can run your render passes.

A solution is to give up on using the implicit transitions and set `initialLayout = layout = finalLayout`. You can then handle the transition with a pipeline barrier, which is easier to use than subpass dependencies and can be declared at command buffer creation time.

You might be wondering if this is an issue with swapchain images as well, when setting `initialLayout = UNDEFINED` and `finalLayout = PRESENT_SRC_KHR`.
The final transition to `PRESENT_SRC_KHR` is safe as long as we pass the `signalSemaphore` from queue submission to `vkQueuePresentKHR`. The initial transition requires further considerations, as explained here: [Transitioning the swapchain image on acquisition](#transitioning-the-swapchain-image-on-acquisition)


## Debugging a DEVICE_LOST

###### Q:

I'm getting a `DEVICE_LOST` error when calling either `vkQueueSubmit` or `vkWaitForFences`. Validation is clean. Why could that be?

###### A:

While developing a complex Vulkan application you might encounter a `VK_ERROR_DEVICE_LOST` after seemingly normal usage. This is relatively expensive to deal with, as it is a sticky error flag and requires the `VkDevice` to be rebuilt, and quite hard to debug too. The Vulkan spec does not currently provide the driver with a straightforward way to communicate the cause of the error, so some trial and error might be required.

There are two main reasons why a `DEVICE_LOST` might arise:

* Out of memory (OOM)
* Resource corruption

We covered OOM conditions in greater detail in [this blog](https://community.arm.com/developer/tools-software/graphics/b/blog/posts/memory-limits-with-vulkan-on-mali-gpus). If your application is within a reasonable vertex budget for mobile (around 2 million vertices under normal usage, as discussed in the blog), it is worth looking for resource corruption due to missing synchronization.

Common signs for synchronization issues are flickering and inconsistencies between devices. An application with incorrect API usage might run fine on some platforms and fail on others. If GPU resources are corrupted due to missing synchronization, a `VK_ERROR_DEVICE_LOST` usually occurs.

It should be noted that missing synchronization does not necessarily result in a lost device. For example, if your rendering pipeline depends on the ordering of render passes you will need to add some synchronization, such as pipeline barriers, between them. Just issuing the render passes in order is not enough to guarantee that they will be executed in order.

Applications with unsynchronized render passes might run as expected in some platforms and show flickering in some others, without any Vulkan errors or validation messages. This is because the API usage is technically correct, but it does not correspond to your intentions.

Synchronization bugs are tricky to identify, reproduce and track down. Validation layers do not cover all cases, but they can help in some situations. Having a mental model of the data dependencies in your rendering pipeline is critical too. An approach to debugging synchronization issues is to temporarily add more synchronization (e.g. extra pipeline barriers, wait idle), in order to narrow down the point where the missing synchronization happens.


## Meaning of srcAccessMask = 0

###### Q:

Does `srcAccessMask = 0` mean "can't read or write"? Shouldn't we specify `VK_ACCESS_COLOR_ATTACHMENT_READ_BIT` in the following code?

```cpp
VkSubpassDependency dependency = { 0 };
dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
dependency.dstSubpass = 0;
dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
// Since we changed the image layout, we need to make the memory visible to
// color attachment to modify.
dependency.srcAccessMask = 0;
dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
```

###### A:

`srcAccessMask = 0` refers to the access scope before the transition happens. There are two reasons that explain why that scope can be 0:

* The image was just acquired from the presentation engine. It is in `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` layout, but we use it as `VK_IMAGE_LAYOUT_UNDEFINED` because we don't need to preserve previous results. Our rendering pipeline doesn't use the image before the transition (no reads or writes), so the access mask can be empty from this point of view.
* We still want to make sure that any memory accesses from the presentation engine are visible after the barrier, but we don't have to specify anything in `srcAccessMask`: the acquisition semaphore already guarantees that any external accesses are made visible when the semaphore is signaled. This is not the case for `dstAccessMask`, in which we need to specify what we want to do with the image (read and write in this case) so the right caches can be flushed.


## Usage of a single image in multiple frames

###### Q:

Why does the multisampling sample (from the [Arm Vulkan SDK](https://github.com/ARM-software/vulkan-sdk)) use a single multisampled color image in multiple frames? The same thing happens with the depth image.

Doesn't this break GPU synchronization?

###### A:

This is due to an optimization for tiled GPUs when you have an attachment that will only be used in a single render pass and doesn't need to be stored. Taking the multisampled render target as an example:

```cpp
// This image will only be used as a transient render target.
// Its purpose is only to hold the multisampled data before resolving the render pass.
info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

[...]

alloc.memoryTypeIndex =
    findMemoryTypeFromRequirementsWithFallback(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
```

With a usage of `TRANSIENT_ATTACHMENT_BIT` and a memory type with `LAZILY_ALLOCATED_BIT` the GPU can avoid allocating the image at all, the data will only reside in tile-local memory. At the end of the render pass the multisampled data are resolved to the swapchain image (see `pResolveAttachments`). Using the multisampled image doesn't involve any actual memory accesses, so we can use the same one for all framebuffers without hazards.

The same reasoning applies to the depth image you mentioned - it's only used in that render pass and never stored, so we can avoid allocating it at all.

This optimization saves a significant amount of memory bandwidth, you can find more information in the tutorial: https://arm-software.github.io/vulkan-sdk/multisampling.html


## Using multiple queues

###### Q:

In which cases should you use more than one queue?

###### A:

Multiple queues could be used for more complex applications such as asynchronous compute. See here: https://gpuopen.com/concurrent-execution-asynchronous-queues/

And this is a mobile-friendly application of async compute: https://community.arm.com/developer/tools-software/graphics/b/blog/posts/using-compute-post-processing-in-vulkan-on-mali


## Changing a buffer's contents dynamically

###### Q:

Can we update a buffer while it is in flight? Do we need barriers to do it?

###### A:

Changing a uniform while the GPU is using it is dangerous from a synchronization standpoint: you cannot know if the GPU will read the data before or after the update, so the behavior of your app would be inconsistent.

The spec says:

> The descriptor set contents bound by a call to vkCmdBindDescriptorSets may be consumed during host execution of the command, or during shader execution of the resulting draws, or any time in between. Thus, the contents must not be altered (overwritten by an update command, or freed) between when the command is recorded and when the command completes executing on the queue. The contents of pDynamicOffsets are consumed immediately during execution of vkCmdBindDescriptorSets. Once all pending uses have completed, it is legal to update and reuse a descriptor set.

If you want to change the uniform buffer data across frames without breaking synchronization, you'll have to replicate those data in some way. One way to do so without major changes to your code would be to create a larger uniform buffer (e.g. 3x the size for 3 frames) and bind it as a dynamic uniform buffer, changing the dynamic offset for each frame.

Since you cannot update a part of a buffer that is in use, pipeline barriers won't help: if you have a single buffer, the update on the CPU side will have to wait for the GPU to finish using the buffer, so you would end up serializing frames.


## Allocating and mapping memory for a buffer

###### Q:

What's the best practice for allocating and mapping buffer memory?

###### A:

Allocating memory for each buffer via `vkAllocateMemory` might be really slow and there's a cap on the total number of allocations, plus mapping memory via `vkMapMemory` is a costly operation.
The intended usage for an app is to allocate a big chunk of memory, keep it mapped and manage it.

If you want a drop-in replacement for memory management which follows these best practices, check out [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator). Its API is similar to Vulkan's so it probably won't require any major changes to your code.


## Understanding barrier scope

###### Q:

Supposing we are only using one queue and we have the following code:

```cpp
// Set of commands - A
vkCmdDraw(...)
...
vkCmdDraw(...)

// Barrier 1
vkCmdPipelineBarrier(...)

// Set of commands - B
vkCmdDraw(...)
...
vkQueueSubmit(...)
vkQueuePresentKHR(...)

// Barrier 2
vkCmdPipelineBarrier(...)

// Set of commands - C
vkCmdDraw(...)
...
vkQueueSubmit(...)
vkQueuePresentKHR(...)
```

How do the two barriers interact with each set of commands?

###### A:

A pipeline barrier always acts on two sets of commands, those which come before the barrier and those which come after.

Since you don't mention render passes we will assume that the calls to `vkCmdPipelineBarrier` are outside of a render pass instance. The spec says:

> If vkCmdPipelineBarrier is called outside a render pass instance, then the first set of commands is all prior commands submitted to the queue and recorded in the command buffer and the second set of commands is all subsequent commands recorded in the command buffer and submitted to the queue.

The main difference between the two barriers is that the first one is in the middle of a command buffer, while the second one is after the first commands are submitted and presented (so it is likely to be in another command buffer).
This difference does not really matter according to the spec, because commands previously submitted and previously recorded in the current command buffer are treated the same way.

This is a breakdown of the 2 barriers:

* Barrier1
    * before: set A and everything that comes prior to that
    * after: sets B, C and everything that comes afterwards
* Barrier2
    * before: sets A, B and everything that comes prior to them
    * after: set C and everything that comes afterwards


## Number of descriptor pools

###### Q:

How many descriptor pools should you have? Just a large one or one per frame?

###### A:

Using one descriptor pool per frame it is not *strictly* necessary but it's still very good to have. If you create your descriptor pool without the `FREE_DESCRIPTOR_SET_BIT` flag, it means you can only free the pool via `vkResetDescriptorPool`.
If you use only a single pool for all frames, you will have to wait idle before freeing. If you use several descriptor pools instead, you'll be able to free them for the frames that are not currently in flight.

Avoiding the `FREE_DESCRIPTOR_SET_BIT` flag can let the driver use a simpler allocator, ultimately improving performance.

You can also check out the [Descriptor management tutorial](samples/performance/descriptor_management/descriptor_management_tutorial.md#descriptor-and-buffer-management) for more information.
If you are performing multithreaded rendering, you may need to allocate more descriptor pools, as discussed in the [Multithreading tutorial](samples/performance/command_buffer_usage/command_buffer_usage_tutorial.md#multi-threaded-recording).


## Synchronizing texture transfers

###### Q:

How should I synchronize texture transfers without calling `vkQueueWaitIdle`? What happens if I don't specify any synchronization?

###### A:

If you don't specify any synchronization there is a concurrency risk: you have no guarantee that the transfer will be complete when the rendering begins. You could add a pipeline barrier between the transfer and the shader stage in which you are going to use the image (you will need a pipeline barrier for the layout transition anyway).

If you're uploading many textures at once, for example when loading a new scene, it might just be easier to submit all the transfers and wait idle.


## Meaning of signaling a fence

###### Q:

When a fence is signaled, does it mean that all commands are transferred to the GPU or that all commands have completed?

###### A:

If it's the fence you get from `vkQueueSubmit`, yes, it means that commands are executed completely.

Actually it means even more than that! If the fence is signaled it means that all commands from all previous submissions are executed completely:

> When a fence is submitted to a queue as part of a queue submission command, it defines a memory dependency on the batches that were submitted as part of that command, and defines a fence signal operation which sets the fence to the signaled state.
>
> The first synchronization scope includes every batch submitted in the same queue submission command. **Fence signal operations that are defined by vkQueueSubmit additionally include in the first synchronization scope all commands that occur earlier in submission order.**


## Struct alignment for uniform buffers and push constants

###### Q:

How do you pass data from a C/C++ struct to a uniform buffer? The data I'm passing is not read correctly from the shader.

###### A:

Uniform buffer alignment is not straightforward due to structure packing rules: a struct in C++ will not match a struct in GLSL unless you structure them carefully. You can find more information on the std140 packing [here](https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Memory_layout), which applies both to uniform buffers and push constants. Debugging it might be hard: if you're lucky validation layers will complain about some offsets you're not expecting, otherwise you'll just see weird values being passed to the shaders.

The golden rule is that struct and array elements must be aligned as multiples of 16 bytes (the size of a `vec4`). Thus:

* `vec4` and `mat4` are safe, feel free to use them
* don't use `vec3`, use a `vec4` and pack some other information in the 4th component if possible
* if you need to use `float` / `int32_t`, you'll need to add a `vec3` of padding after them; try to pack basic types in a `vec4` whenever possible

Dynamic uniform buffers have an additional alignment requirement for the dynamic offset, so you might need to further pad your uniform buffer data so that the offset is an exact multiple of that limit. You can check the limit as `minUniformBufferOffsetAlignment` in `VkPhysicalDeviceProperties`, with common values ranging between 16 and 256 bytes.


## Crashes with no backtrace on Android

###### Q:

My Android app crashes without any message or backtrace on logcat. What could it be?

###### A:

Your app may be running out of memory. Look for a message like this in logcat:

```
07-13 17:10:37.788 19132 19132 V threaded_app: LowMemory: 0x7926307ec0
```

If you are running out of memory, debugging the app in Android Studio Profiler may help, as it lets you track the memory usage of your app and may let you trace it down to individual allocations.


## Shader variants

###### Q:

How can I set up shader variants in Vulkan? Should I use specialization constants?

###### A:

A first approach to shader variants is to use `#ifdef` directives in your shaders, like in [this one](https://github.com/KhronosGroup/glTF-WebGL-PBR/blob/master/shaders/pbr-vert.glsl). You can then compile different variants by running `glslangValidator` with the `-D` option, like this:

```cpp
%VULKAN_SDK%\bin\glslangValidator.exe -V pbr.vert -o variants\pbr_vert_.spv
%VULKAN_SDK%\bin\glslangValidator.exe -V pbr.vert -o variants\pbr_vert_N.spv -DHAS_NORMALS
%VULKAN_SDK%\bin\glslangValidator.exe -V pbr.vert -o variants\pbr_vert_T.spv -DHAS_TANGENTS
%VULKAN_SDK%\bin\glslangValidator.exe -V pbr.vert -o variants\pbr_vert_NT.spv -DHAS_NORMALS -DHAS_TANGENTS
```

This can be done either at compile time or at runtime, by building `glslang` along with your app.

A different approach to shader variants is to use specialization constants: they are efficient as they are still compile-time constants, specified at pipeline creation time, and you don't need to compile separate variants with `glslangValidator` or `shaderc`. Specialization constants do have some limitations, however, the main one being that you can't use `if` statements while defining your shader's interface, like vertex attributes, texture samplers:

```cpp
// valid GLSL
#ifdef HAS_BASECOLORMAP
    layout(binding = 0) uniform texture2D baseColorT;
#endif


// invalid GLSL
if (specialization_constant) {
    layout(binding = 0) uniform texture2D baseColorT;
}
```

So the interface for your shaders will be fixed, but you can use if statements based on specialization constants in your `main()` function, which will be evaluated at compile time just like `#define`. Even if you can't modify the shader interface variables, the compiler may optimize out the ones you don't need, if you remove all references to them.


## Multithreaded rendering performance

###### Q:

I set up multithreaded rendering but it's running slower than single threaded. What could be going on?

###### A:

Multithreaded command submission has the potential to improve CPU time significantly, but it also opens up several pitfalls which in the worst case can lead to worse performance than single threaded.

Our general recommendation is to use a profiler and figure out the bottleneck for your application, while keeping a close eye on common pain points with threading in general.
These are the issues that we have encountered more often:

* Thread spawning can cause a significant overhead; this could happen if you use `std::async` directly to spawn your threads, as STL implementations usually do not pool threads in that case. We recommend using a thread pool library instead, or to implement thread pooling yourself.

* Synchronization overhead might be significant. If you are using mutexes to guard all your map accesses, the code might end up running in a serialized fashion with the extra overhead for lock acquisition/release. Alternative approaches could be to use a read/write mutex like [`std::shared_mutex`](https://en.cppreference.com/w/cpp/thread/shared_mutex), or to go lock-free by ensuring that the map is read-only while executing multithreaded code.
In the lock-free approach, each thread can keep a list of entries to add to the map; these per-thread lists of entries are then inserted into the map after all the threads have returned.

* Having few meshes per thread. Multithreaded command recording has some performance overhead both on the CPU side (cost of threading) and on the GPU side (executing secondary command buffers), so using the full parallelism available is not always a good choice. As a rule of thumb, only go parallel if you measure that draw call recording is taking a significant portion of your frame time.
