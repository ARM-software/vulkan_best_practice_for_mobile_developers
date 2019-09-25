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

# Miscellaneous

## Controls

| Action                   |                     Desktop                     |               Mobile               |
| :----------------------- | :---------------------------------------------: | :--------------------------------: |
| Move around the scene    |                 <kbd>WASD</kbd>                 | <kbd>press</kbd> + <kbd>hold</kbd> |
| Rotate camera            | <kbd>right mouse button</kbd> + <kbd>drag</kbd> | <kbd>press</kbd> + <kbd>move</kbd> |
| Pan                      | <kbd>left mouse button</kbd> + <kbd>drag</kbd>  |                 -                  |
| Movement speed reduced   |                <kbd>shift</kbd>                 |                 -                  |
| Movement speed increased |                 <kbd>ctrl</kbd>                 |                 -                  |
| toggle GUI               |              <kbd>left click</kbd>              |           <kbd>tap</kbd>           |
| toggle Debug Window      |             <kbd>right click</kbd>              |      2 finger <kbd>tap</kbd>       |

## Debug Window

The Debug Window shows information about the current application.

![Debug Window](./images/debug-window.png)

## Driver version

The debug window shows the driver version of the GPU, which follows the [Vulkan semantics](https://vulkan.lunarg.com/doc/view/1.0.26.0/linux/vkspec.chunked/ch02s09.html), with a major, minor, and patch number. New versions of the driver will increment these numbers.

The framework is able to get these values by calling `vkGetPhysicalDeviceProperties(physical_device, &properties)`. This will set all attributes of properties, which type is `struct VkPhysicalDeviceProperties`. Then it extracts the relevant bits from `properties.driverVersion` using the following [Vulkan macros](https://vulkan.lunarg.com/doc/view/1.0.26.0/linux/vkspec.chunked/apds03.html#boilerplate-versions):

```c
VK_VERSION_MAJOR(properties.driverVersion);
VK_VERSION_MINOR(properties.driverVersion);
VK_VERSION_PATCH(properties.driverVersion);
```

It is important to note that old Arm Mali drivers (before Bifrost r14 and Midgard r26) may not implement this feature, therefore the values returned will be undefined.
