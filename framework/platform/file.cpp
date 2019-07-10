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

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
VKBP_ENABLE_WARNINGS()

namespace vkb::file
{
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
	return read_binary_file(Path::assets() + filename, count);
}

std::vector<uint8_t> read_temp(const std::string &filename, const uint32_t count)
{
	return read_binary_file(Path::temp() + filename, count);
}

void write_temp(const std::vector<uint8_t> &data, const std::string &filename, const uint32_t count)
{
	write_binary_file(data, Path::temp() + filename, count);
}

void write_image(const uint8_t *data, const std::string &filename, const uint32_t width, const uint32_t height, const uint32_t components, const uint32_t row_stride)
{
	stbi_write_png((Path::storage() + "/" + filename + ".png").c_str(), width, height, components, data, row_stride);
}

const std::string &Path::assets()
{
	static std::string asset_path = get_asset_path();
	if (asset_path.empty())
	{
		throw std::runtime_error("Platform must initialize the asset path for file::Path");
	}
	return asset_path;
}

const std::string &Path::temp()
{
	static std::string temp_path = get_temp_path();
	if (temp_path.empty())
	{
		throw std::runtime_error("Platform must initialize the temporary path for file::Path");
	}
	return temp_path;
}

const std::string &Path::storage()
{
	static std::string storage_path = get_storage_path();
	if (storage_path.empty())
	{
		throw std::runtime_error("Platform must initialize the storage path for file::Path");
	}
	return storage_path;
}

const std::string &Path::logs()
{
	static std::string logs_path = get_logs_path();
	if (logs_path.empty())
	{
		throw std::runtime_error("Platform must initialize the logs path for file::Path");
	}
	return logs_path;
}

}        // namespace vkb::file
