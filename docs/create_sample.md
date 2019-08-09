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

# Creating a new sample <!-- omit in toc -->

This document will explain how to create the necessary files to start working on your own sample.

- [Creating](#create)
- [Configuring](#configure)

## Create
To create a new sample, run the `generate_sample` script that exists within `bldsys/scripts`. There is a batch script for Windows, and a shell script for Unix based systems. 

#### Usage

```
generate_sample <sample_id> <category> [<create_path>]
```

* `<sample_id>` is the id tag that your sample will have (e.g. '`my_new_sample`')
* `<category>` is the category this sample will be placed into (e.g. '`advanced`')
* `<create_path>` is optional, for deciding *where* your sample gets created. This should generally be left blank as the script will automatically place your sample its category folder.

#### Example

```
generate_sample my_sample category
```

Running the above line will generate the following files:

```
samples/category/my_sample/CMakeLists.txt
samples/category/my_sample/my_sample.cpp
samples/category/my_sample/my_sample.h
```

## Configure
To configure how your sample will appear, you can modify the `CMakeLists.txt` within the generated directory:
* `NAME` the string that will appear as the title of the sample in the sample list, and the header in the GUI for the sample
* `DESCRIPTION` the string that will appear under the samples title in the sample list

To change the order of the samples (or place your sample in a specific place), modify the `ORDER_LIST` list inside `samples/CMakeLists.txt`. Just place a string of your `<sample_id>` where you would like it to be placed relative to the other samples.

If you would like to show different configurations of your sample during batch mode, you will need to insert these configurations in the constructor of your sample (inside `samples/category/my_sample.cpp`).

e.g. `get_configuration().insert<vkb::IntSetting>(0, my_sample_value, 3);`