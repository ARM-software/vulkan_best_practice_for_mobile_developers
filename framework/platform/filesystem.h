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

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

#include <json.hpp>

namespace vkb
{
namespace fs
{
namespace path
{
enum Type
{
	//Relative paths
	Assets,
	Shaders,
	Storage,
	Screenshots,
	Logs,
	Graphs,
	/* NewFolder */
	TotalRelativePathTypes,

	//Special paths
	ExternalStorage,
	WorkingDir = ExternalStorage,
	Temp
};

extern const std::unordered_map<Type, std::string> relative_paths;

/**
 * @brief Gets the path of a given type
 * @param type The type of path to get
 * @throws runtime_error if the platform didn't initialize each path properly, path wasn't found or the path was found but is empty
 * @return Path to the directory of a certain type
 */
const std::string get(const Type type);
}        // namespace path

/**
 * @brief Helper to tell if a given path is a directory
 * @param path A path to a directory
 * @return True if the path points to a valid directory, false if not
 */
bool is_directory(const std::string &path);

/**
 * @brief Platform specific implementation to create a directory
 * @param path A path to a directory
 */
void create_directory(const std::string &path);

/**
 * @brief Recursively creates a directory
 * @param root The root directory that the path is relative to
 * @param path A path in the format 'this/is/an/example/path/'
 */
void create_path(const std::string &root, const std::string &path);

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
 * @brief Helper to read a shader file into a byte-array
 *
 * @param filename The path to the file (relative to the assets directory)
 * @return A vector filled with data read from the file
 */
std::vector<uint8_t> read_shader(const std::string &filename);

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
void write_image(const uint8_t *data, const std::string &filename, const uint32_t width, const uint32_t height, const uint32_t components, const uint32_t row_stride);

/**
 * @brief Helper to output a json graph
 * 
 * @param data A json object
 * @param filename The name of the file
 */
bool write_json(nlohmann::json &data, const std::string &filename);
}        // namespace fs
}        // namespace vkb
