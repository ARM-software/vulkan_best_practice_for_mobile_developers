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

#include "render_pipeline.h"

#include "scene_graph/components/camera.h"
#include "scene_graph/components/image.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/pbr_material.h"
#include "scene_graph/components/sampler.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/node.h"

namespace vkb
{
RenderPipeline::RenderPipeline(std::vector<std::unique_ptr<Subpass>> &&subpasses_) :
    subpasses{std::move(subpasses_)}
{
	for (auto &subpass : subpasses)
	{
		subpass->prepare();
	}

	// Default clear values
	clear_value[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
	clear_value[1].depthStencil = {0.0f, ~0U};

	load_store[1].store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

void RenderPipeline::add_subpass(std::unique_ptr<Subpass> &&subpass)
{
	subpass->prepare();
	subpasses.emplace_back(std::move(subpass));
}

std::vector<std::unique_ptr<Subpass>> &RenderPipeline::get_subpasses()
{
	return subpasses;
}

const std::vector<LoadStoreInfo> &RenderPipeline::get_load_store() const
{
	return load_store;
}

void RenderPipeline::set_load_store(const std::vector<LoadStoreInfo> &ls)
{
	load_store = ls;
}

const std::vector<VkClearValue> &RenderPipeline::get_clear_value() const
{
	return clear_value;
}

void RenderPipeline::set_clear_value(const std::vector<VkClearValue> &cv)
{
	clear_value = cv;
}

void RenderPipeline::draw(CommandBuffer &command_buffer, RenderTarget &render_target, VkSubpassContents contents)
{
	assert(!subpasses.empty() && "Render pipeline should contain at least one sub-pass");

	// Pad clear values if they're less than render target attachments
	while (clear_value.size() < render_target.get_attachments().size())
	{
		clear_value.push_back({0.0f, 0.0f, 0.0f, 1.0f});
	}

	for (size_t i = 0; i < subpasses.size(); ++i)
	{
		active_subpass_index = i;

		auto &subpass = subpasses[i];

		subpass->update_render_target_attachments();

		if (i == 0)
		{
			command_buffer.begin_render_pass(render_target, load_store, clear_value, subpasses, contents);
		}
		else
		{
			command_buffer.next_subpass();
		}

		subpass->draw(command_buffer);
	}

	active_subpass_index = 0;
}

std::unique_ptr<Subpass> &RenderPipeline::get_active_subpass()
{
	return subpasses[active_subpass_index];
}
}        // namespace vkb
