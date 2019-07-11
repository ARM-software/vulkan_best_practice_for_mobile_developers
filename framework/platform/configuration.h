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
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace vkb
{
class Setting
{
  public:
	Setting() = default;

	Setting(Setting &&other) = default;

	virtual ~Setting()
	{}

	virtual void set() = 0;

	virtual std::type_index get_type() = 0;
};

class BoolSetting : public Setting
{
  public:
	BoolSetting(bool &handle, bool value);

	virtual void set() override;

	virtual std::type_index get_type() override;

  private:
	bool &handle;

	bool value;
};

class IntSetting : public Setting
{
  public:
	IntSetting(int &handle, int value);

	virtual void set() override;

	virtual std::type_index get_type() override;

  private:
	int &handle;

	int value;
};

class EmptySetting : public Setting
{
  public:
	EmptySetting();

	virtual void set() override;

	virtual std::type_index get_type() override;
};

using ConfigMap = std::map<uint32_t, std::unordered_map<std::type_index, std::vector<Setting *>>>;

/**
 * @brief A class that contains configuration data for a sample.
 */
class Configuration
{
  public:
	/**
	 * @brief Configures the settings in the current config
	 */
	void set();

	/**
	 * @brief Increments the configuration count
	 * @returns True if the current configuration iterator was incremented
	 */
	bool next();

	/**
	 * @brief Resets the configuration to beginning
	 */
	void reset();

	/**
	 * @brief Inserts a setting into the current configuration
	 * @param config_index The configuration to insert the setting into
	 * @param setting A setting to be inserted into the configuration
	 */
	void insert_setting(uint32_t config_index, std::unique_ptr<Setting> setting);

	/**
	 * @brief Inserts a setting into the current configuration
	 * @param config_index The configuration to insert the setting into
	 * @param args A parameter pack containing the parameters to initialize a setting object
	 */
	template <class T, class... A>
	void insert(uint32_t config_index, A &&... args)
	{
		static_assert(std::is_base_of<Setting, T>::value,
		              "T is not a type of setting.");

		insert_setting(config_index, std::make_unique<T>(args...));
	}

  protected:
	ConfigMap configs;

	std::vector<std::unique_ptr<Setting>> settings;

	ConfigMap::iterator current_configuration;
};
}        // namespace vkb
