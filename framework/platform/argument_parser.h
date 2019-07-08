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

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace vkb
{
/**
 * @brief Splits a string into a vector by a single char delimiter
 * @param s String to split
 * @param delim Single character to split the string by
 * @return A vector of the components within a string
 */
std::vector<std::string> split(const std::string &s, char delim);

/**
 * @brief Class that handles and formats arguments passed into the application
 */
class ArgumentParser
{
  public:
	/**
	 * @brief Constructor
	 * @param argument_string The full string of arguments passed to the program
	 */
	ArgumentParser(const std::string argument_string);

	/**
	 * @brief Helper function to tell the existance of a flag
	 * @param argument The flag to check for
	 * @returns True if the flag exists within the argument map, false if not
	 */
	bool contains(const std::string &argument) const;

	/**
	 * @brief Helper function to return the value from a flag
	 * @param argument The flag to get the value from
	 * @returns A string of the value 
	 */
	const std::string at(const std::string &argument) const;

	/**
	 * @brief Helper function to return the vector of values from a flag
	 * @param argument The flag to get the vector from
	 * @returns A vector of strings supplied after the flag
	 */
	const std::vector<std::string> get(const std::string &argument) const;

  private:
	std::unordered_map<std::string, std::vector<std::string>> arguments;
};
}        // namespace vkb
