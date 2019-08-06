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

#include "platform/filesystem.h"

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
VKBP_ENABLE_WARNINGS()

#include "platform/platform.h"

namespace vkb
{
namespace fs
{
namespace path
{
const std::unordered_map<Type, std::string> relative_paths = {{Type::Assets, "assets/"},
                                                              {Type::Storage, "output/"},
                                                              {Type::Screenshots, "output/images/"},
                                                              {Type::Logs, "output/logs/"}};

const std::string get(const Type type)
{
	assert(relative_paths.size() == Type::TotalRelativePathTypes && "Not all paths are defined in filesystem, please check that each enum is specified");

	// Check for special cases first
	if (type == Type::WorkingDir)
	{
		return Platform::get_external_storage_directory();
	}
	else if (type == Type::Temp)
	{
		return Platform::get_temp_directory();
	}

	// Check for relative paths
	auto it = relative_paths.find(type);

	if (relative_paths.size() < Type::TotalRelativePathTypes)
	{
		throw std::runtime_error("Platform hasn't initialized the paths correctly");
	}
	else if (it == relative_paths.end())
	{
		throw std::runtime_error("Path enum doesn't exist, or wasn't specified in the path map");
	}
	else if (it->second.empty())
	{
		throw std::runtime_error("Path was found, but it is empty");
	}

	auto path = Platform::get_external_storage_directory() + it->second;

	if (!is_directory(path))
	{
		create_path(Platform::get_external_storage_directory(), it->second);
	}

	return path;
}
}        // namespace path

bool is_directory(const std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
	{
		return false;
	}
	else if (info.st_mode & S_IFDIR)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void create_path(const std::string &root, const std::string &path)
{
	for (auto it = path.begin(); it != path.end(); ++it)
	{
		it = std::find(it, path.end(), '/');
		create_directory(root + std::string(path.begin(), it));
	}
}

static std::vector<uint8_t> read_binary_file(const std::string &filename, const uint32_t count)
{
	std::vector<uint8_t> data;

	std::ifstream file;

	file.open(filename, std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	uint64_t read_count = count;
	if (count == 0)
	{
		file.seekg(0, std::ios::end);
		read_count = static_cast<uint64_t>(file.tellg());
		file.seekg(0, std::ios::beg);
	}

	data.resize(static_cast<size_t>(read_count));
	file.read(reinterpret_cast<char *>(data.data()), read_count);
	file.close();

	return data;
}

static void write_binary_file(const std::vector<uint8_t> &data, const std::string &filename, const uint32_t count)
{
	std::ofstream file;

	file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	uint64_t write_count = count;
	if (count == 0)
	{
		write_count = data.size();
	}

	file.write(reinterpret_cast<const char *>(data.data()), write_count);
	file.close();
}

std::vector<uint8_t> read_asset(const std::string &filename, const uint32_t count)
{
	return read_binary_file(path::get(path::Type::Assets) + filename, count);
}

std::vector<uint8_t> read_temp(const std::string &filename, const uint32_t count)
{
	return read_binary_file(path::get(path::Type::Temp) + filename, count);
}

void write_temp(const std::vector<uint8_t> &data, const std::string &filename, const uint32_t count)
{
	write_binary_file(data, path::get(path::Type::Temp) + filename, count);
}

void write_image(const uint8_t *data, const std::string &filename, const uint32_t width, const uint32_t height, const uint32_t components, const uint32_t row_stride)
{
	stbi_write_png((path::get(path::Type::Screenshots) + filename + ".png").c_str(), width, height, components, data, row_stride);
}
}        // namespace fs
}        // namespace vkb
