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

#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

namespace vkb::file
{
/**
 * @brief Helper to tell if a given path is a directory
 *
 * @param path A path to a directory
 * @return True if the path points to a valid directory, false if not
 */
bool is_directory(const std::string &path);

/**
 * @brief Helper to read an asset file into a byte-array
 *
 * @param filename The path to the file (relative to the assets directory)
 * @param count (optional) How many bytes to read. If 0 or not specified, the size
 * of the file will be used.
 * @return A vector filled with data read from the file
 */
std::vector<uint8_t> read_asset(const std::string &filename, const uint32_t count = 0);

/**
 * @brief Helper to read a temporary file into a byte-array
 *
 * @param filename The path to the file (relative to the temporary storage directory)
 * @param count (optional) How many bytes to read. If 0 or not specified, the size
 * of the file will be used.
 * @return A vector filled with data read from the file
 */
std::vector<uint8_t> read_temp(const std::string &filename, const uint32_t count = 0);

/**
 * @brief Helper to write to a file in temporary storage
 *
 * @param data A vector filled with data to write
 * @param filename The path to the file (relative to the temporary storage directory)
 * @param count (optional) How many bytes to write. If 0 or not specified, the size
 * of data will be used.
 */
void write_temp(const std::vector<uint8_t> &data, const std::string &filename, const uint32_t count = 0);

/**
 * @brief Helper to write to a png image in permanent storage
 *
 * @param data       A vector filled with pixel data to write in (R, G, B, A) format
 * @param filename   The name of the image file without an extension
 * @param width      The width of the image
 * @param height     The height of the image
 * @param components The number of bytes per element
 * @param row_stride The stride in bytes of a row of pixels
 */
void write_image(const std::vector<uint8_t> &data, const std::string &filename, const uint32_t width, const uint32_t height, const uint32_t components, const uint32_t row_stride);

/**
 * @brief Manages initialization of platform-dependent file paths
 */
class Path
{
  public:
	/**
	 * @brief Platform dependent, for use with asset files
	 * @return Path to the assets directory
	 */
	static const std::string &assets();

	/**
	 * @brief Platform dependent, for use with temporary files
	 * @return Path to temporary storage
	 */
	static const std::string &temp();

	/**
	 * @brief Platform dependent, for use with storage files
	 * @return Path to permanent storage
	 */
	static const std::string &storage();

  private:
	static std::string get_asset_path();

	static std::string get_temp_path();

	static std::string get_storage_path();
};

}        // namespace vkb::file
