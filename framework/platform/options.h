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

#include <docopt.h>

#include "common/logging.h"

namespace vkb
{
/**
 * @brief Class that handles and formats arguments passed into the application
 */
class Options
{
  public:
	Options() = default;

	/**
	 * @brief Parses the arguments, forcing an exit if it fails
	 * @param usage The usage string of the application
	 * @param args The arguments supplied to the program
	 */
	void parse(const std::string &usage, const std::vector<std::string> &args);

	/**
	 * @brief Helper function that determines if key exists within parsed args
	 * @param argument The argument to check for
	 * @returns True if the argument exists in the parsed_arguments
	 */
	bool contains(const std::string &argument) const;

	/**
	 * @brief Helper function to return the integer value from a flag
	 * @param argument The flag to check for
	 * @returns An integer representation of the value under the argument
	 */
	const int32_t get_int(const std::string &argument) const;

	/**
	 * @brief Helper function to return the string value from a flag
	 * @param argument The flag to check for
	 * @returns A string representation of the value under the argument
	 */
	const std::string get_string(const std::string &argument) const;

	/**
	 * @brief Prints a formatted usage of the arguments
	 */
	void print_usage() const;

  private:
	std::string usage;

	std::map<std::string, docopt::value> parse_result;
};
}        // namespace vkb
