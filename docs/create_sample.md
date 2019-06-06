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

# Create a Sample <!-- omit in toc -->

- [Generate](#generate)
- [Configure](#configure)

## Generate
If you would like to create a new sample the following script will generate a sample within the `samples` directory with a given name `MySample`

##### Windows <!-- omit in toc -->

```
bldsys/scripts/generate_sample.bat <sample_name> <category>
```

##### Linux <!-- omit in toc -->

```
./bldsys/scripts/generate_sample.sh <sample_name> <category>
```

The sample is now created and the following files generated

```
samples/category/sample_name/CMakeLists.txt
samples/category/sample_name/sample_name.cpp
samples/category/sample_name/sample_name.h
```

## Configure
To configure how your sample will appear in the android list activity you can change the `CMakeLists.txt` within the generated directory:
* `NAME` the string that will appear as the title of the sample in the sample list, and the header in the GUI for the sample
* `DESCRIPTION` the string that will appear under the samples title in the sample list

Inside `samples/CMakeLists.txt` you should place your `<sample_name>` where you would like it to be placed relative to the other samples.

Within the samples constructor, you will need to insert the various configurations that you want your sample to show when automated by the `Run All Samples` button.

e.g. `get_configuration().insert<vkb::IntSetting>(0, my_sample_value, 3);`