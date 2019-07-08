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

# Testing Guides

## Contents 
- [System Test](#system-test)

## System Test
In order for the script to work you will need to install and add to your Path:
* `Python 3.x`
* `imagemagick`
* `git`
* `cmake` 
* (Optional) `adb` if you plan to use Android

You will also need to have built the Vulkan Best Practices in 64 bit, with the CMake flag `VKB_BUILD_TESTS`, `VKB_ENTRYPOINTS` and `VKB_SYMLINKS` set to `ON`, and in addition to this install a working `.apk` onto a device if you plan on testing on Android.

**Before you begin a system test on Android, ensure that the device is held in landscape, and there isn't an instance of Vulkan Best Practice running already.**

1. From the root of the project: `cd tests/system_test`
2. To run: `python system_test.py -B <build dir> -H <root_dir> -C <Debug|Release>`  
2.1. e.g. `python system_test.py -Bbuild/windows -H../.. -CRelease` (build path is relative to root)  
2.2. To target just testing on desktop, add a `-D` flag, or to target just Android, an `-A` flag. If no flag is specified it will run for both.  
2.3. To run a specific sub test(s), use the `-S` flag (e.g. `python system_test.py ... -S sponza bonza` runs sponza and bonza)  
2.4. To use a different image comparison metric, use the `-M` flag, by default [MAE](https://en.wikipedia.org/wiki/Mean_absolute_error) is used.

## Android

We currently support FHD resolutions (2280x1080), if testing on another device or resolution the test may fail.