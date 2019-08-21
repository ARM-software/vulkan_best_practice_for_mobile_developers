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

#include <nlohmann/json.hpp>

#include "common/error.h"

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
template <class NodeType>
class Graph
{
  public:
	Graph(const char *name_);

	/**
	 * @brief add a node to the graph and pass its id
	 * All Node types must have at least two parameters: size_t id, T node
	 * @param node 
	 */
	void add_node(NodeType node);

	/**
	 * @brief Create a node object
	 * @tparam T
	 * @tparam Args 
	 * @param node required
	 * @param args 
	 * @return size_t id of the node in the graph
	 */
	template <typename T, typename... Args>
	size_t create_node(const T &node, Args... args)
	{
		const void *uid;
		uid     = reinterpret_cast<const void *>(&node);
		auto it = uids.find(uid);
		if (it != uids.end())
		{
			return it->second;
		}

		size_t id = get_id();
		uids[uid] = id;
		nodes[id] = NodeType{id, node, args...};
		return id;
	}

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
	 * @param file to dump to
	 */
	void dump_to_file(std::string file);

	size_t get_id();

  private:
	size_t                                   next_id = 0;
	std::vector<Edge>                        adj;
	std::unordered_map<size_t, NodeType>     nodes;
	std::unordered_map<const void *, size_t> uids;
	std::string                              name;
};

template <class T>
Graph<T>::Graph(const char *name_)
{
	name = name_;
}

template <class T>
size_t Graph<T>::get_id()
{
	return next_id++;
}

template <class NodeType>
void Graph<NodeType>::add_node(NodeType node)
{
	size_t id = get_id();
	nodes.insert({id, {node}});
}

template <class NodeType>
void Graph<NodeType>::add_edge(size_t from, size_t to)
{
	auto it = std::find_if(adj.begin(), adj.end(), [from, to](auto &e) -> bool { return e.from == from && e.to == to; });
	if (it == adj.end())
	{
		adj.push_back({get_id(), from, to});
	}
}

template <class NodeType>
void Graph<NodeType>::remove_edge(size_t from, size_t to)
{
	auto it = std::find_if(adj.begin(), adj.end(), [from, to](auto &e) -> bool { return e.from == from && e.to == to; });
	if (it != adj.end())
	{
		adj.erase(it);
	}
}

template <class NodeType>
void Graph<NodeType>::dump_to_file(std::string file)
{
	std::vector<nlohmann::json> edges;
	for (auto &e : adj)
	{
		auto it = nodes.find(e.from);
		if (it != nodes.end())
		{
			e.options["group"] = it->second.attributes["group"];
		}
		e.options["id"]     = e.id;
		e.options["source"] = e.from;
		e.options["target"] = e.to;
		edges.push_back({{"data", e.options}});
	}

	std::vector<nlohmann::json> node_json;
	auto                        it = nodes.begin();
	while (it != nodes.end())
	{
		node_json.push_back(it->second.attributes);
		it++;
	}

	nlohmann::json j = {
	    {"name", name},
	    {"nodes", node_json},
	    {"edges", edges}};

	std::ofstream o;

	o.open(file, std::ios::out | std::ios::trunc);

	if (o.good())
	{
		// Whitespace needed as last character is overritten on android causing the json to be corrupt
		o << j << " ";
	}
	else
	{
		LOGE("Error outputting graph");
	};

	o.close();
}
}        // namespace utils
}        // namespace vkb