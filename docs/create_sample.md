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
bldsys/scripts/generate_sample.bat MySample
```

##### Linux <!-- omit in toc -->

```
./bldsys/scripts/generate_sample.sh MySample
```

The sample is now created and the following files generated

```
samples/my_sample/CMakeLists.txt
samples/my_sample/my_sample.cpp
samples/my_sample/my_sample.h
```

## Configure
To configure how your sample will appear in the android list activity you can change the `samples/my_sample/CMakeLists.txt` `NAME` and `DESCRIPTION`
