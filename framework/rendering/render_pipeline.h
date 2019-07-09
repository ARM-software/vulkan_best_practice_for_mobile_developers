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

#include "common/helpers.h"
#include "core/buffer.h"
#include "rendering/render_frame.h"
#include "rendering/subpass.h"
#include "utils.h"

namespace vkb
{
/**
 * @brief A RenderPipeline is a sequence of Subpass objects.
 * The most basic one is SceneSubpass.
 * Subpass holds shaders and can draw the core::sg::Scene.
 * SceneSubpass holds a GlobalUniform for uniform data across all meshes.
 * More subpasses can be added to the sequence if required.
 * For example, postprocessing can be implemented with two pipelines which
 * share render targets.
 */
class RenderPipeline : public NonCopyable
{
  public:
	RenderPipeline(std::vector<std::unique_ptr<Subpass>> &&subpasses = {});

	virtual ~RenderPipeline() = default;

	RenderPipeline(RenderPipeline &&other) = default;

	/**
	 * @return Load store info
	 */
	const std::vector<LoadStoreInfo> &get_load_store() const;

	/**
	 * @param load_store Load store info to set
	 */
	void set_load_store(std::vector<LoadStoreInfo> &load_store);

	/**
	 * @return Clear values
	 */
	const std::vector<VkClearValue> &get_clear_value() const;

	/**
	 * @param clear_values Clear values to set
	 */
	void set_clear_value(std::vector<VkClearValue> &clear_values);

	/**
	 * @brief Appends a subpass to the pipeline
	 * @param subpass Subpass to append
	 */
	void add_subpass(std::unique_ptr<Subpass> &&subpass);

	/**
	 * @brief Record draw commands for each Subpass
	 */
	void draw(CommandBuffer &command_buffer, RenderTarget &render_target);

  private:
	std::vector<std::unique_ptr<Subpass>> subpasses;

	/// Default to two load store
	std::vector<LoadStoreInfo> load_store = std::vector<LoadStoreInfo>(2);

	/// Default to two clear values
	std::vector<VkClearValue> clear_value = std::vector<VkClearValue>(2);
};
}        // namespace vkb
