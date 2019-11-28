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

import sys, os, platform, subprocess, shutil

generate_sample_script = "bldsys/scripts/generate_sample"
script_path            = os.path.dirname(os.path.realpath(__file__))
root_path              = os.path.join(script_path, "../../")

def add_sample():
    generate_sample_path = os.path.join(root_path, generate_sample_script) 
    if platform.system() == "Windows":
        generate_sample_path += ".bat"
    else:
        generate_sample_path += ".sh"

    result = True
    try:
        subprocess.run(generate_sample_path, cwd=root_path)
    except FileNotFoundError:
        print("Error: Couldn't find generate sample script")
        result = False
    except:
        print("Error: Couldn't generate sample, script failed")
        result = False
    return result

def clear(platform):
    try:
        os.remove("build/" + platform + "/CMakeCache.txt")
    except OSError:
        pass

def build():
    clear("windows")
    clear("linux")
    clear("mac")

    result = True
    generate_command = ""
    build_command = ""
    if platform.system() == "Windows":
        generate_command = "cmake -G\"Visual Studio 15 2017 Win64\" -H. -Bbuild/windows -DVKB_BUILD_SAMPLES=ON -DVKB_SYMLINKS=OFF"
        build_command = "cmake --build build/windows --config Release --target vulkan_best_practice"
    elif platform.system() == "Linux":
        generate_command = "cmake -G \"Unix Makefiles\" -H. -Bbuild/linux -DCMAKE_BUILD_TYPE=Release -DVKB_BUILD_SAMPLES=ON -DVKB_SYMLINKS=OFF"
        build_command = "cmake --build build/linux --config Release --target vulkan_best_practice"
    elif platform.system() == "Darwin":
        generate_command = "cmake -H. -Bbuild/mac -DCMAKE_BUILD_TYPE=Release -DVKB_BUILD_SAMPLES=ON -DVKB_SYMLINKS=OFF"
        build_command = "cmake --build build/mac --config Release --target vulkan_best_practice"
    else:
        print("Error: Platform not supported")
        return False

    try:
        process = subprocess.run(generate_command, cwd=root_path, shell=True)
        process = subprocess.run(build_command, cwd=root_path, shell=True)
        if process.returncode != 0:
            return False
    except:
        print("Error: Build process failed")
        result = False

    return result

if __name__ == "__main__":
    result = False
    if(add_sample() and build()):
        result = True

    sample_path = os.path.join(root_path, "samples/api/sample_test")
    if os.listdir(sample_path):
        shutil.rmtree(sample_path)

    if result:
        print("Generate Sample Test: Passed")
        exit(0)
    else:
        print("Generate Sample Test: Failed")
        exit(1)
        
    
