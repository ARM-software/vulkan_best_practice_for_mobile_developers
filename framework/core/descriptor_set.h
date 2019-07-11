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
#include "common/vk_common.h"

namespace vkb
{
class Device;
class DescriptorSetLayout;

class DescriptorSet : public NonCopyable
{
  public:
	DescriptorSet(Device &                                  device,
	              DescriptorSetLayout &                     descriptor_set_layout,
	              const BindingMap<VkDescriptorBufferInfo> &buffer_infos = {},
	              const BindingMap<VkDescriptorImageInfo> & image_infos  = {});

	DescriptorSet(DescriptorSet &&other);

	~DescriptorSet();

	void update(const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
	            const BindingMap<VkDescriptorImageInfo> & image_infos);

	VkDescriptorSet get_handle() const;

  private:
	Device &device;

	DescriptorSetLayout &descriptor_set_layout;

	VkDescriptorSet handle{VK_NULL_HANDLE};
};
}        // namespace vkb
