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

#include "windows_platform.h"

#include <Windows.h>
#include <iostream>
#include <shellapi.h>
#include <stdexcept>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
VKBP_ENABLE_WARNINGS()

#include "platform/glfw_window.h"
#include "platform/headless_window.h"

namespace vkb
{
namespace
{
inline const std::string get_temp_path_from_environment()
{
	std::string temp_path = "temp/";

	TCHAR temp_buffer[MAX_PATH];
	DWORD temp_path_ret = GetTempPath(MAX_PATH, temp_buffer);
	if (temp_path_ret > MAX_PATH || temp_path_ret == 0)
	{
		temp_path = "temp/";
	}
	else
	{
		temp_path = std::string(temp_buffer) + "/";
	}

	return temp_path;
}

/// @brief Converts wstring to string using Windows specific function
/// @param wstr Wide string to convert
/// @return A converted utf8 string
std::string wstr_to_str(const std::wstring &wstr)
{
	if (wstr.empty())
	{
		return {};
	}

	auto wstr_len = static_cast<int>(wstr.size());
	auto str_len  = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr_len, NULL, 0, NULL, NULL);

	std::string str(str_len, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr_len, &str[0], str_len, NULL, NULL);

	return str;
}

inline std::vector<std::string> get_args()
{
	LPWSTR *argv;
	int     argc;

	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	// Ignore the first argument containing the application full path
	std::vector<std::wstring> arg_strings(argv + 1, argv + argc);
	std::vector<std::string>  args;

	for (auto &arg : arg_strings)
	{
		args.push_back(wstr_to_str(arg));
	}

	return args;
}
}        // namespace

namespace fs
{
void create_directory(const std::string &path)
{
	if (!is_directory(path))
	{
		CreateDirectory(path.c_str(), NULL);
	}
}
}        // namespace fs

WindowsPlatform::WindowsPlatform(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                                 PSTR lpCmdLine, INT nCmdShow) :
    DesktopPlatform(get_args(), get_temp_path_from_environment())
{
	if (!AllocConsole())
	{
		throw std::runtime_error{"AllocConsole error"};
	}

	FILE *fp;
	freopen_s(&fp, "conin$", "r", stdin);
	freopen_s(&fp, "conout$", "w", stdout);
	freopen_s(&fp, "conout$", "w", stderr);
}

void WindowsPlatform::terminate(ExitCode code)
{
	Platform::terminate(code);

	if (code != ExitCode::Success || benchmark_mode)
	{
		std::cout << "Press enter to close...\n";
		std::cin.get();
	}

	FreeConsole();
}

const char *WindowsPlatform::get_surface_extension()
{
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}
}        // namespace vkb
