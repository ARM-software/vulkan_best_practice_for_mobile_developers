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

#include "platform/file.h"

namespace vkb::file
{
std::string Path::get_asset_path()
{
	static std::string assets = "assets/";

	if (!is_directory(assets))
	{
		mkdir(assets.c_str(), 0777);
	}

	return assets;
}

std::string Path::get_temp_path()
{
	std::string temp_path = "/tmp/";
	if (const char *env_ptr = std::getenv("TMPDIR"))
	{
		temp_path = std::string(env_ptr) + "/";
	}
	return temp_path;
}

std::string Path::get_storage_path()
{
	static std::string storage = "output/";

	if (!is_directory(storage))
	{
		mkdir(storage.c_str(), 0777);
	}

	return storage;
}
}        // namespace vkb::file
