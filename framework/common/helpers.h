/* Copyright (c) 2018-2019, Arm Limited and Contributors
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
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/error.h"

VKBP_DISABLE_WARNINGS()
#include "common/glm_common.h"
#include <glm/gtx/hash.hpp>
VKBP_ENABLE_WARNINGS()

namespace vkb
{
template <typename T>
inline void read(std::istringstream &is, T &value)
{
	is.read(reinterpret_cast<char *>(&value), sizeof(T));
}

inline void read(std::istringstream &is, std::string &value)
{
	std::size_t size;
	read(is, size);
	value.resize(size);
	is.read(const_cast<char *>(value.data()), size);
}

template <class T>
inline void read(std::istringstream &is, std::set<T> &value)
{
	std::size_t size;
	read(is, size);
	for (uint32_t i = 0; i < size; i++)
	{
		T item;
		is.read(reinterpret_cast<char *>(&item), sizeof(T));
		value.insert(std::move(item));
	}
}

template <class T>
inline void read(std::istringstream &is, std::vector<T> &value)
{
	std::size_t size;
	read(is, size);
	value.resize(size);
	is.read(reinterpret_cast<char *>(value.data()), value.size() * sizeof(T));
}

template <class T, class S>
inline void read(std::istringstream &is, std::map<T, S> &value)
{
	std::size_t size;
	read(is, size);

	for (uint32_t i = 0; i < size; i++)
	{
		std::pair<T, S> item;
		read(is, item.first);
		read(is, item.second);

		value.insert(std::move(item));
	}
}

template <class T, uint32_t N>
inline void read(std::istringstream &is, std::array<T, N> &value)
{
	is.read(reinterpret_cast<char *>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void read(std::istringstream &is, T &first_arg, Args &... args)
{
	read(is, first_arg);

	read(is, args...);
}

template <typename T>
inline void write(std::ostringstream &os, const T &value)
{
	os.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

inline void write(std::ostringstream &os, const std::string &value)
{
	write(os, value.size());
	os.write(value.data(), value.size());
}

template <class T>
inline void write(std::ostringstream &os, const std::set<T> &value)
{
	write(os, value.size());
	for (const T &item : value)
	{
		os.write(reinterpret_cast<const char *>(&item), sizeof(T));
	}
}

template <class T>
inline void write(std::ostringstream &os, const std::vector<T> &value)
{
	write(os, value.size());
	os.write(reinterpret_cast<const char *>(value.data()), value.size() * sizeof(T));
}

template <class T, class S>
inline void write(std::ostringstream &os, const std::map<T, S> &value)
{
	write(os, value.size());

	for (const std::pair<T, S> &item : value)
	{
		write(os, item.first);
		write(os, item.second);
	}
}

template <class T, uint32_t N>
inline void write(std::ostringstream &os, const std::array<T, N> &value)
{
	os.write(reinterpret_cast<const char *>(value.data()), N * sizeof(T));
}

template <typename T, typename... Args>
inline void write(std::ostringstream &os, const T &first_arg, const Args &... args)
{
	write(os, first_arg);

	write(os, args...);
}

/**
 * @brief Helper function to combine a given hash
 *        with a generated hash for the input param.
 */
template <class T>
inline void hash_combine(size_t &seed, const T &v)
{
	std::hash<T> hasher;
	glm::detail::hash_combine(seed, hasher(v));
}

/**
 * @brief Helper function to convert a data type
 *        to string using output stream operator.
 * @param value The object to be converted to string
 * @return String version of the given object
 */
template <class T>
inline std::string to_string(const T &value)
{
	std::stringstream ss;
	ss << std::fixed << value;
	return ss.str();
}

/**
 * @brief Helper function to check size_t is correctly converted to uint32_t
 * @param value Value of type @ref size_t to convert
 * @return An @ref uint32_t representation of the same value
 */
template <class T>
uint32_t to_u32(T value)
{
	static_assert(std::is_arithmetic<T>::value, "T must be numeric");

	if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<uint32_t>::max()))
	{
		throw std::runtime_error("to_u32() failed, value is too big to be converted to uint32_t");
	}

	return static_cast<uint32_t>(value);
}

}        // namespace vkb
