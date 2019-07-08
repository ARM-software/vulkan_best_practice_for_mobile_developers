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

import sys, os, math, platform, threading, datetime, subprocess, zipfile, argparse, shutil, struct, imghdr
from time import sleep

# Settings (changing these may cause instabilities)
dependencies      = ("magick", "cmake", "git", "adb")
sub_tests         = []
test_desktop      = True
test_android      = True
comparison_metric = "MAE"
current_dir       = os.getcwd()
build_path        = ""
root_path         = ""
build_config      = ""
outputs_path      = "output/"
tmp_path          = "tmp/"
archive_path      = "artifacts/"
image_ext         = ".png"
android_timeout   = 60 # How long in seconds should we wait before timing out on Android
check_step        = 5
threshold         = 0.9999 # How similar the images are allowed to be before they pass

class Subtest:
    result = False
    test_name = ""
    platform = ""

    def __init__(self, test_name, platform):
        self.test_name = test_name
        self.platform = platform

    def run(self, path):
        result = True
        working_dir = os.path.dirname(os.path.realpath(__file__)) + "/" + root_path
        path = working_dir + path
        try:
            subprocess.run([path, "--offscreen"], cwd=working_dir)
        except FileNotFoundError:
            print("\t\t\t(Error) Couldn't find application ({})".format(path))
            result = False
        except:
            print("\t\t\t(Error) Application error ({})".format(path))
            result = False
        return result

    def test(self):
        print("\t\t=== Test started: {} ===".format(self.test_name))
        self.result = True
        screenshot_path = tmp_path + self.platform + "/"
        try:
            shutil.move(root_path + outputs_path + self.test_name + image_ext, screenshot_path + self.test_name + image_ext)
        except FileNotFoundError:
            print("\t\t\t(Error) Couldn't find screenshot ({}), perhaps test crashed".format(root_path + outputs_path + self.test_name + image_ext))
            self.result = False
            return
        if not test(self.test_name, screenshot_path):
            self.result = False

    def passed(self):
        if self.result:
            print("\t\t=== Passed! ===")
        else:
            print("\t\t=== Failed. ===")
        return self.result

class WindowsSubtest(Subtest):
    def __init__(self, test_name):
        super().__init__(test_name, "Windows")

    def run(self):
        return super().run("{}tests/system_test/sub_tests/{}/bin/{}/{}/{}.exe".format(build_path, self.test_name, build_config, platform.machine(), self.test_name + "_test"))

class LinuxSubtest(Subtest):
    def __init__(self, test_name):
        super().__init__(test_name, "Linux")

    def run(self):
        return super().run("{}tests/system_test/sub_tests/{}/bin/{}/{}/{}".format(build_path, self.test_name, build_config, platform.machine(), self.test_name))

class AndroidSubtest(Subtest):
    def __init__(self, test_name):
        super().__init__(test_name, "Android")

    def run(self):
        subprocess.run("adb shell am force-stop com.arm.vulkan_best_practice")
        subprocess.run(["adb", "shell", "am", "start", "-W", "-n", "com.arm.vulkan_best_practice/com.arm.vulkan_best_practice.BPSampleActivity", "-e", "test", "{0}".format(self.test_name)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        output = subprocess.check_output("adb shell dumpsys window windows | grep -E 'mCurrentFocus|mFocusedApp' | cut -d . -f 5 | cut -d ' ' -f 1")
        activity = "".join(output.decode("utf-8").split())
        timeout_counter = 0
        while activity == "vulkan_best_practice" and timeout_counter <= android_timeout:
            sleep(check_step)
            timeout_counter += check_step
            output = subprocess.check_output("adb shell \"dumpsys window windows | grep -E 'mCurrentFocus|mFocusedApp' | cut -d . -f 5 | cut -d ' ' -f 1\"")
            activity = "".join(output.decode("utf-8").split())
        if timeout_counter <= android_timeout:
            subprocess.run(["adb", "pull", "/sdcard/Android/data/com.arm.vulkan_best_practice/files/" + outputs_path + self.test_name + image_ext, root_path + outputs_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return True
        else:
            print("\t\t\t(Error) Timed out")
            return False

def create_app(platform, test_name):
    """
    @brief   Creates a buildable and runnable test, returning it
    @param   platform An integer representing what platform the app should be built for
    @param   test_name The name of the test, used to create the app
    @return  A runnable application
    """
    if platform == "Windows":
        return WindowsSubtest(test_name)
    elif platform == "Linux":
        return LinuxSubtest(test_name)
    elif platform == "Android":
        return AndroidSubtest(test_name)
    else:
        print("Error: cannot create subtest, cant find associated platform.")
        exit(1)

def get_command(command):
    """
    @brief  Ensures command can be executed on each platform
    @param  command The commands name
    @return A platform appropriate command
    """
    if platform.system() == "Windows":
        command += ".exe"
    return command

def get_resolution(image):
    """
    @brief   Gets the width and height of a given image
    @param   image The path to the image relative to this script
    @return  A string denoting the resolution in the format (WxH)
    """
    return subprocess.check_output([get_command("magick"), "identify", "-format", "\"%[fx:w]x%[fx:h]\"", image]).decode("utf-8")[1:-1]

def compare(metric, base_image, test_image, diff_image = "null:"):
    """
    @brief   Compares two images by their mean absolute error (changing the order of these parameters will change the contents of diff_image)
    @param   metric     The type of image comparison you want to invoke
    @param   base_image The relative path to the image to base the test on
    @param   test_image The relative path to compare the base_image with
    @param   diff_image The relative path to the output image of the difference between the two images, default "null:"
    @return  A float clamped between 0 and 1 denoting how similar the images are (1 being identical, 0 being opposite)
    """
    output = ""
    try:
        output = subprocess.check_output([get_command("magick"), "compare", "-metric", metric, base_image, test_image, diff_image], stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError as e:
        output = e.output
        pass
    output = output.decode("utf-8")
    return max(0.0, min(1.0 - float(output[output.find("(")+1:output.find(")")]), 1.0))

def test(test_name, screenshot_path):
    """
    @brief   Tests each screenshot within the tmp/ folder against the goldtest, saving the results if it fails
    @param   test_name       The name of the test, used to retrieve the respective goldtest image
    @param   screenshot_path The directory where to store screenshots
    @return  True if the image tests pass
    """
    # Run test
    result = False
    image = test_name + image_ext
    base_image = screenshot_path + image
    test_image = "gold/{0}/{1}.png".format(test_name, get_resolution(base_image))
    if not os.path.isfile(test_image):
        print("\t\t\t(Error) Resolution not supported, gold image not found ({})".format(test_image))
        return False
    diff_image = "{0}{1}-diff.png".format(screenshot_path, image[0:image.find(".")])
    print("\t\t\t(Comparing images...) '{0}' with '{1}':".format(base_image, test_image), end = " ", flush = True)
    similarity = compare(comparison_metric, base_image, test_image, diff_image)
    print("{}%".format(100*math.floor(similarity*10000)/10000))
    # Remove images if it is identical
    if similarity >= threshold:
        os.remove(base_image)
        os.remove(diff_image)
        result = True
    return result

def main():
    """
    @brief Runs the system test
    """
    if test_android and not os.path.exists(tmp_path + "Android/"):
        os.makedirs(tmp_path + "Android/")
    if test_desktop and not os.path.exists(tmp_path + platform.system()):
        os.makedirs(tmp_path + platform.system())

    print("=== System Test started! ===")
    results = []

    # Create tests
    apps = []
    for test_name in sub_tests:
        if test_android:
            apps.append(create_app("Android", test_name))
        if test_desktop:
            apps.append(create_app(platform.system(), test_name))

    # Run tests
    for app in apps:
        if app:
            print("\t=== Running {} on {} ===".format(app.test_name, app.platform))
            if app.run():
                app.test()
                results.append(app.passed())
            else:
                results.append(False)

    # Evaluate system test

    passed = 0
    failed = 0

    for result in results:
        if result:
            passed += 1
        else:
            failed += 1

    total_result = True
    if failed > 0:
        total_result = False

    if total_result:
        print("=== Success: All tests passed! ===")        
        shutil.rmtree(tmp_path)
        exit(0)
    else:
        print("=== Failed: {} passed - {} failed ===".format(passed, failed))
        # If the screenshot directory isn't empty, create an archive of the results
        if os.listdir(tmp_path) is not None:
            print("=== Archiving results into '{}' ===".format(shutil.make_archive(archive_path + "system_test" + "-" + datetime.datetime.now().strftime("%Y.%m.%d-%H.%M.%S"), 'zip', tmp_path)))
        shutil.rmtree(tmp_path)
        exit(1)

if __name__ == "__main__":
    argparser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter, description="A simple script that builds, runs, screenshots, and tests your apps against a pre-existing gold")
    argparser.add_argument("-B", "--build", required=True, help="relative path to the cmake build directory")
    argparser.add_argument("-H", "--home", required=True, help="relative path to the root working directory")
    argparser.add_argument("-C", "--config", required=True, help="build configuration to use")
    argparser.add_argument("-M", "--metric", default=comparison_metric, help="the image comparison metric to use (for list see https://imagemagick.org/script/command-line-options.php#metric)")
    argparser.add_argument("-S", "--subtests", default=os.listdir("sub_tests"), nargs="+", help="if set the specified sub tests will be run instead")
    build_group = argparser.add_mutually_exclusive_group()
    build_group.add_argument("-D", "--desktop", action='store_false', help="flag to only deploy tests on desktop")
    build_group.add_argument("-A", "--android", action='store_false', help="flag to only deploy tests on android")

    args = vars(argparser.parse_args())
    build_path    = args["build"]
    root_path     = args["home"]
    build_config  = args["config"]
    metric        = args["metric"]
    sub_tests     = args["subtests"]
    test_desktop  = args["android"]
    test_android  = args["desktop"]

    if build_path[-1] != "/":
        build_path += "/"
    if root_path[-1] != "/":
        root_path += "/"

    # Ensure the right dependencies are installed before continuing
    runnable = True
    for dependency in dependencies:
        if shutil.which(dependency) is None:
            print("Error: {} not installed".format(dependency))
            runnable = False
    if not runnable:
        exit(1)

    # If building for android check that a valid device is plugged in
    if test_android:
        try:
            subprocess.check_output("adb get-state")
        except:
            print("Device not found, disabling Android testing")
            test_android = False
        else:
            print("Device found!")

    # Run script and handle keyboard interruption
    try:
        main()
    except KeyboardInterrupt:
        print("System Test Aborted")
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
