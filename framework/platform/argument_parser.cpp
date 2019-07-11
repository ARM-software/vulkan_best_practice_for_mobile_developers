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

#include "argument_parser.h"

namespace vkb
{
std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> tokens;
	std::string              str = s;
	while (str.size())
	{
		auto index = str.find(delim);
		if (index != std::string::npos)
		{
			tokens.push_back(str.substr(0, index));
			str = str.substr(index + 1);
			if (str.size() == 0)
			{
				tokens.push_back(str);
			}
		}
		else
		{
			tokens.push_back(str);
			str = "";
		}
	}
	return tokens;
}

ArgumentParser::ArgumentParser(const std::string argument_string)
{
	auto split_args = split(argument_string, '-');

	for (auto &a : split_args)
	{
		if (!a.empty())
		{
			//For each first first word after the appearance of a hyphen ('-'),
			// insert every word after it til the next hyphen in the form of a vector
			auto i = a.find(" ");
			arguments.insert({a.substr(0, i), split(a.substr(i + 1, a.size()), ' ')});
		}
	}
}

bool ArgumentParser::contains(const std::string &argument) const
{
	if (arguments.find(argument) != arguments.end())
	{
		return true;
	}

	return false;
}

const std::string ArgumentParser::at(const std::string &argument) const
{
	auto it = arguments.find(argument);
	if (it != arguments.end())
	{
		if (it->second.size() > 0)
		{
			return *it->second.begin();
		}
	}

	return "";
}

const std::vector<std::string> ArgumentParser::get(const std::string &argument) const
{
	auto it = arguments.find(argument);
	if (it != arguments.end())
	{
		if (it->second.size() > 0)
		{
			return it->second;
		}
	}

	return {};
}
}        // namespace vkb
