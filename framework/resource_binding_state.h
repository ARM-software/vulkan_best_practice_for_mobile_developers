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

#include "common/vk_common.h"
#include "core/buffer.h"
#include "core/image_view.h"
#include "core/sampler.h"

namespace vkb
{
/**
 * @brief A resource info is a struct containing the actual resource data.
 *
 * This will be referenced by a buffer info or image info descriptor inside a descriptor set.
 */
struct ResourceInfo
{
	bool dirty{false};

	const core::Buffer *buffer{nullptr};

	VkDeviceSize offset{0};

	VkDeviceSize range{0};

	const core::ImageView *image_view{nullptr};

	const core::Sampler *sampler{nullptr};
};

/**
 * @brief A resource set is a set of bindings containing resources that were bound 
 *        by a command buffer.
 *
 * The ResourceSet has a one to one mapping with a DescriptorSet.
 */
class ResourceSet
{
  public:
	void reset();

	bool is_dirty() const;

	void clear_dirty();

	void clear_dirty(uint32_t binding, uint32_t array_element);

	void bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t array_element);

	void bind_image(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t binding, uint32_t array_element);

	void bind_input(const core::ImageView &image_view, uint32_t binding, uint32_t array_element);

	const BindingMap<ResourceInfo> &get_resource_bindings() const;

  private:
	bool dirty{false};

	BindingMap<ResourceInfo> resource_bindings;
};

/**
 * @brief The resource binding state of a command buffer.
 *
 * Keeps track of all the resources bound by the command buffer. The ResourceBindingState is used by
 * the command buffer to create the appropriate descriptor sets when it comes to draw.
 */
class ResourceBindingState
{
  public:
	void reset();

	bool is_dirty();

	void clear_dirty();

	void clear_dirty(uint32_t set);

	void bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_image(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_input(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);

	const std::unordered_map<uint32_t, ResourceSet> &get_resource_sets();

  private:
	bool dirty{false};

	std::unordered_map<uint32_t, ResourceSet> resource_sets;
};
}        // namespace vkb
