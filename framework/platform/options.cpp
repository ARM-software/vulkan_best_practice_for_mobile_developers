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

#include "options.h"

#include <sstream>

namespace vkb
{
void Options::parse(const std::string &usage, const std::vector<std::string> &args)

{
	if (usage.size() != 0)
	{
		this->usage = usage;

		if (args.size() > 0)
		{
			this->parse_result = docopt::docopt(usage, args, false);
		}
	}
}

bool Options::contains(const std::string &argument) const
{
	if (parse_result.count(argument) != 0)
	{
		if (const auto &result = parse_result.at(argument))
		{
			if (result.isBool())
			{
				return result.asBool();
			}

			return true;
		}
	}

	return false;
}

const int32_t Options::get_int(const std::string &argument) const
{
	if (contains(argument))
	{
		auto result = parse_result.at(argument);
		if (result.isString())
		{
			return std::stoi(result.asString());
		}
		else if (result.isLong())
		{
			return static_cast<int32_t>(result.asLong());
		}

		throw std::runtime_error("Argument option is not int type");
	}

	throw std::runtime_error("Couldn't find argument option");
}

const std::string Options::get_string(const std::string &argument) const
{
	if (contains(argument))
	{
		auto result = parse_result.at(argument);
		if (result.isString())
		{
			return result.asString();
		}

		throw std::runtime_error("Argument option is not string type");
	}

	throw std::runtime_error("Couldn't find argument option");
}

void Options::print_usage() const
{
	if (!usage.empty())
	{
		std::stringstream sstream(usage);
		std::string       token;
		while (std::getline(sstream, token, '\n'))
		{
			LOGI(token);
		}
	}
}
}        // namespace vkb
