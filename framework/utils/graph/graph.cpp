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

#include "graph.h"

#include "platform/filesystem.h"

namespace vkb
{
namespace utils
{
Graph::Graph(const char *new_name) :
    name(new_name)
{
}

size_t Graph::new_id()
{
	return next_id++;
}

void Graph::add_edge(size_t from, size_t to)
{
	auto it = std::find_if(adj.begin(), adj.end(), [from, to](auto &e) -> bool { return e.from == from && e.to == to; });
	if (it == adj.end())
	{
		adj.push_back({new_id(), from, to});
	}
}

void Graph::remove_edge(size_t from, size_t to)
{
	auto it = std::find_if(adj.begin(), adj.end(), [from, to](auto &e) -> bool { return e.from == from && e.to == to; });
	if (it != adj.end())
	{
		adj.erase(it);
	}
}

size_t Graph::create_vk_image(const VkImage &image)
{
	const void *addr = reinterpret_cast<const void *>(image);
	const void *uid  = get_uid(addr);
	if (!uid)
	{
		auto id    = create_vk_node("VkImage", image);
		uids[addr] = id;
		return id;
	}
	return reinterpret_cast<size_t>(uid);
}

size_t Graph::create_vk_image_view(const VkImageView &image)
{
	const void *addr = reinterpret_cast<const void *>(image);
	const void *uid  = get_uid(addr);
	if (!uid)
	{
		auto id    = create_vk_node("VkImageView", image);
		uids[addr] = id;
		return id;
	}
	return reinterpret_cast<size_t>(uid);
}

const void *Graph::get_uid(const void *addr)
{
	auto it = uids.find(addr);
	if (it != uids.end())
	{
		return reinterpret_cast<const void *>(it->second);
	}
	return nullptr;
}

bool Graph::dump_to_file(std::string file)
{
	std::vector<nlohmann::json> edges;
	for (auto &e : adj)
	{
		auto it = nodes.find(e.from);
		if (it != nodes.end())
		{
			e.options["group"] = it->second->attributes["group"];
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
		node_json.push_back(it->second->attributes);
		it++;
	}

	nlohmann::json j = {
	    {"name", name},
	    {"nodes", node_json},
	    {"edges", edges}};

	return fs::write_json(j, file);
}

}        // namespace utils
}        // namespace vkb