#!/bin/bash
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


ROOT_DIR="$(cd "$(dirname "$0")"; pwd)/../.."

if [ "$#" -lt 1 ]; then
    SAMPLE_NAME=SampleTest
else
    SAMPLE_NAME="$1"
fi

if [ "$#" -lt 2 ]; then
    CATEGORY=api
else
    CATEGORY="$2"
fi

if [ "$#" -lt 3 ]; then
    BUILD_DIR=$ROOT_DIR/samples/$CATEGORY
else
    BUILD_DIR="$3"
fi

cmake -DSAMPLE_NAME=$SAMPLE_NAME \
      -DOUTPUT_DIR=$BUILD_DIR \
      -P $ROOT_DIR/bldsys/cmake/create_sample_project.cmake
