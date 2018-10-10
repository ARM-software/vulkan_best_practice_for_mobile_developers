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

#include "resource_binding_state.h"

namespace vkb
{
void ResourceBindingState::reset()
{
	clear_dirty();

	set_bindings.clear();
}

bool ResourceBindingState::is_dirty()
{
	return dirty;
}

void ResourceBindingState::clear_dirty()
{
	dirty = false;
}

void ResourceBindingState::clear_dirty(uint32_t set)
{
	set_bindings[set].clear_dirty();
}

void ResourceBindingState::bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element)
{
	set_bindings[set].bind_buffer(buffer, offset, range, binding, array_element);

	dirty = true;
}

void ResourceBindingState::bind_image(const ImageView &image_view, VkSampler sampler, uint32_t set, uint32_t binding, uint32_t array_element)
{
	set_bindings[set].bind_image(image_view, sampler, binding, array_element);

	dirty = true;
}

const std::unordered_map<uint32_t, SetBindings> &ResourceBindingState::get_set_bindings()
{
	return set_bindings;
}

VkDescriptorImageInfo ResourceInfo::get_image_info() const
{
	VkDescriptorImageInfo image_info{};

	image_info.sampler   = sampler;
	image_info.imageView = image_view->get_handle();

	return image_info;
}

const ImageView &ResourceInfo::get_image_view() const
{
	return *image_view;
}

void ResourceInfo::reset()
{
	clear_dirty();

	buffer = nullptr;

	image_view = nullptr;

	sampler = VK_NULL_HANDLE;

	offset = 0;

	range = 0;
}

bool ResourceInfo::is_dirty() const
{
	return dirty;
}

void ResourceInfo::clear_dirty()
{
	dirty = false;
}

bool ResourceInfo::is_buffer() const
{
	return buffer != nullptr;
}

bool ResourceInfo::is_sampler_only() const
{
	return image_view == nullptr && sampler != VK_NULL_HANDLE;
}

bool ResourceInfo::is_image_only() const
{
	return image_view != nullptr && sampler == VK_NULL_HANDLE;
}

bool ResourceInfo::is_image_sampler() const
{
	return image_view != nullptr && sampler != VK_NULL_HANDLE;
}

void ResourceInfo::bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range)
{
	this->buffer = &buffer;

	this->offset = offset;

	this->range = range;

	dirty = true;
}

void ResourceInfo::bind_image(const ImageView &image_view, VkSampler sampler)
{
	this->image_view = &image_view;

	this->sampler = sampler;

	dirty = true;
}

VkDescriptorBufferInfo ResourceInfo::get_buffer_info() const
{
	VkDescriptorBufferInfo buffer_info{};

	buffer_info.buffer = buffer->get_handle();
	buffer_info.offset = offset;
	buffer_info.range  = range;

	return buffer_info;
}

void SetBindings::reset()
{
	clear_dirty();

	resource_bindings.clear();
}

bool SetBindings::is_dirty() const
{
	return dirty;
}

void SetBindings::clear_dirty()
{
	dirty = false;
}

void SetBindings::clear_dirty(uint32_t binding, uint32_t array_element)
{
	resource_bindings[binding][array_element].clear_dirty();
}

void SetBindings::bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t array_element)
{
	resource_bindings[binding][array_element].bind_buffer(buffer, offset, range);

	dirty = true;
}

void SetBindings::bind_image(const ImageView &image_view, VkSampler sampler, uint32_t binding, uint32_t array_element)
{
	resource_bindings[binding][array_element].bind_image(image_view, sampler);

	dirty = true;
}

const BindingMap<ResourceInfo> &SetBindings::get_resource_bindings() const
{
	return resource_bindings;
}

}        // namespace vkb
