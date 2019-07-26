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

#include "configuration.h"

namespace vkb
{
BoolSetting::BoolSetting(bool &handle, bool value) :
    handle{handle},
    value{value}
{
}

void BoolSetting::set()
{
	handle = value;
}

std::type_index BoolSetting::get_type()
{
	return typeid(BoolSetting);
}

IntSetting::IntSetting(int &handle, int value) :
    handle{handle},
    value{value}
{
}

void IntSetting::set()
{
	handle = value;
}

std::type_index IntSetting::get_type()
{
	return typeid(IntSetting);
}

EmptySetting::EmptySetting()
{
}

void EmptySetting::set()
{
}

std::type_index EmptySetting::get_type()
{
	return typeid(EmptySetting);
}

void Configuration::set()
{
	for (auto pair : current_configuration->second)
	{
		for (auto setting : pair.second)
		{
			setting->set();
		}
	}
}

bool Configuration::next()
{
	if (configs.size() == 0)
	{
		return false;
	}

	current_configuration++;

	if (current_configuration == configs.end())
	{
		return false;
	}

	return true;
}

void Configuration::reset()
{
	current_configuration = configs.begin();
}

void Configuration::insert_setting(uint32_t config_index, std::unique_ptr<Setting> setting)
{
	settings.push_back(std::move(setting));
	configs[config_index][settings.back()->get_type()].push_back(settings.back().get());
}

}        // namespace vkb
