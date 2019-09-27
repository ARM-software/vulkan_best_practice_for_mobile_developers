 #!/usr/bin/env python
'''
 Copyright (c) 2019, Arm Limited and Contributors

 SPDX-License-Identifier: MIT

 Permission is hereby granted, free of charge,
 to any person obtaining a copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
'''

from __future__ import print_function
from io import BytesIO
from zipfile import ZipFile

import os, requests

assets_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../assets")

if __name__ == "__main__":
    url = "https://github.com/ARM-software/vulkan_best_practice_for_mobile_developers/releases/download/v1.2.0/scenes.zip"

    try:
        request = requests.get(url, stream=True)
        request.raise_for_status()
    except requests.exceptions.RequestException as e:
        print("Failed to download assets: {0}".format(e))
        exit(1)

    zip_file = ZipFile(BytesIO(request.content))

    try:
        zip_file.extractall(path=assets_path)
    except Exception as e:
        zip_file.close()
        print("Error unpacking file: {0}".format(e))
        exit(1)
        
    zip_file.close()

    print("Successfully unpacked assets at {0}".format(assets_path))
    exit(0)
