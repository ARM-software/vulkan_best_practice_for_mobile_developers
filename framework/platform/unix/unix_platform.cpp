/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "unix_platform.h"

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
VKBP_ENABLE_WARNINGS()

#include "platform/glfw_window.h"
#include "platform/headless_window.h"

#ifndef VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#	define VK_MVK_MACOS_SURFACE_EXTENSION_NAME "VK_MVK_macos_surface"
#endif

#ifndef VK_KHR_XCB_SURFACE_EXTENSION_NAME
#	define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
#endif

namespace vkb
{
namespace
{
inline const std::string get_temp_path_from_environment()
{
	std::string temp_path = "/tmp/";

	if (const char *env_ptr = std::getenv("TMPDIR"))
	{
		temp_path = std::string(env_ptr) + "/";
	}

	return temp_path;
}
}        // namespace

namespace fs
{
void create_directory(const std::string &path)
{
	if (!is_directory(path))
	{
		mkdir(path.c_str(), 0777);
	}
}
}        // namespace fs

UnixPlatform::UnixPlatform(const UnixType &type, int argc, char **argv) :
    DesktopPlatform({argv + 1, argv + argc}, get_temp_path_from_environment()),
    type{type}
{
}

const char *UnixPlatform::get_surface_extension()
{
	if (type == UnixType::Mac)
	{
		return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
	}
	else
	{
		return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
	}
}
}        // namespace vkb
