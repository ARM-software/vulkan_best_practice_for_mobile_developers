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
#include <fstream>
#include <iostream>
#include <vector>

#include <json.hpp>

#include "common/error.h"
#include "node.h"

namespace vkb
{
namespace utils
{
struct Edge
{
	size_t         id;
	size_t         from;
	size_t         to;
	nlohmann::json options;
	Edge(size_t id_, size_t s, size_t t) :
	    id(id_),
	    from(s),
	    to(t){};
};

/**
 * @brief Graph is an implementation of an adjacency list graph. The nodes are created from a variadic function and their implementation is defined by the given NodeType
 * 
 * @tparam NodeType either FrameworkNode or SceneNode
 */
class Graph
{
  public:
	Graph(const char *name);

	/**
	 * @brief Create a node object
	 * @tparam T
	 * @tparam Args 
	 * @param node required
	 * @param args 
	 * @return size_t id of the node in the graph
	 */
	template <class NodeType, typename T, typename... Args>
	size_t create_node(const T &node, Args... args)
	{
		const void *addr = reinterpret_cast<const void *>(&node);
		const void *uid  = get_uid(addr);
		if (!uid)
		{
			size_t id = new_id();
			uids[uid] = id;
			nodes[id] = std::make_unique<NodeType>(id, node, args...);
			return id;
		}
		return reinterpret_cast<size_t>(addr);
	}

	size_t create_vk_image(const VkImage &image);

	size_t create_vk_image_view(const VkImageView &image);

	template <typename T>
	size_t create_vk_node(const char *name, const T &handle)
	{
		size_t id = new_id();
		nodes[id] = std::make_unique<Node>(id, name, "Vulkan", nlohmann::json{{name, Node::handle_to_uintptr_t(handle)}});
		return id;
	}

	/**
	 * @brief Get the uid of a node
	 * 
	 * @param addr 
	 * @return const void* if null node doesnt exist
	 */
	const void *get_uid(const void *addr);

	/**
	 * @brief Add an edge to the graph
	 * @param from source node
	 * @param to target node
	 */
	void add_edge(size_t from, size_t to);

	/**
	 * @brief Remove Edge from the graph
	 * @param from source node
	 * @param to target node
	 */
	void remove_edge(size_t from, size_t to);

	/**
	 * @brief Dump the graphs state to json in the given file name
	 * @param file_name to dump to
	 */
	bool dump_to_file(std::string file_name);

	size_t new_id();

  private:
	size_t                                            next_id = 0;
	std::vector<Edge>                                 adj;
	std::unordered_map<size_t, std::unique_ptr<Node>> nodes;
	std::unordered_map<const void *, size_t>          uids;
	std::string                                       name;
};
}        // namespace utils
}        // namespace vkb