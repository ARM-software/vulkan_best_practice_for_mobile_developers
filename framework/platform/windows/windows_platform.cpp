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
#include <shellapi.h>
#include <stdexcept>

namespace vkb
{
WindowsPlatform::WindowsPlatform(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
                                 PSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
	if (!AllocConsole())
	{
		throw std::runtime_error{"AllocConsole error"};
	}

	FILE *fp;
	freopen_s(&fp, "conin$", "r", stdin);
	freopen_s(&fp, "conout$", "w", stdout);
	freopen_s(&fp, "conout$", "w", stderr);

	LPWSTR *szArgList;
	int     argCount;

	szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);

	// Ignore the first argument containing the application full path
	std::vector<std::wstring> arguments_w(szArgList + 1, szArgList + argCount);

	std::string argument_string = "";

	for (auto &arg_w : arguments_w)
	{
		argument_string += std::string(arg_w.begin(), arg_w.end()) + " ";
	}

	parse_arguments(argument_string);
}

bool WindowsPlatform::initialize(std::unique_ptr<Application> &&app)
{
	return GlfwPlatform::initialize(std::move(app));
}

void WindowsPlatform::terminate()
{
	FreeConsole();

	GlfwPlatform::terminate();
}
}        // namespace vkb
