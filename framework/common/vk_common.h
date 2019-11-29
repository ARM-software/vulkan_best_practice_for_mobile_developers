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

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <vk_mem_alloc.h>
#include <volk.h>

template <class T>
using ShaderStageMap = std::map<VkShaderStageFlagBits, T>;

template <class T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;

namespace vkb
{
/**
 * @brief Helper function to determine if a Vulkan format is depth only.
 * @param format Vulkan format to check.
 * @return True if format is a depth only, false otherwise.
 */
bool is_depth_only_format(VkFormat format);

/**
 * @brief Helper function to determine if a Vulkan format is depth or stencil.
 * @param format Vulkan format to check.
 * @return True if format is a depth or stencil, false otherwise.
 */
bool is_depth_stencil_format(VkFormat format);

/**
 * @brief Helper function to determine if a Vulkan descriptor type is a dynamic storage buffer or dynamic uniform buffer.
 * @param descriptor_type Vulkan descriptor type to check.
 * @return True if type is dynamic buffer, false otherwise.
 */
bool is_dynamic_buffer_descriptor_type(VkDescriptorType descriptor_type);

/**
 * @brief Helper function to determine if a Vulkan descriptor type is a buffer (either uniform or storage buffer, dynamic or not).
 * @param descriptor_type Vulkan descriptor type to check.
 * @return True if type is buffer, false otherwise.
 */
bool is_buffer_descriptor_type(VkDescriptorType descriptor_type);

/**
 * @brief Helper function to get the bits per pixel of a Vulkan format.
 * @param format Vulkan format to check.
 * @return The bits per pixel of the given format, -1 for invalid formats.
 */
int32_t get_bits_per_pixel(VkFormat format);

/**
 * @brief Helper function to convert a VkFormat enum to a string
 * @param format Vulkan format to convert.
 * @return The string to return.
 */
const std::string convert_format_to_string(VkFormat format);

/**
 * @brief Image memory barrier structure used to define
 *        memory access for an image view during command recording.
 */
struct ImageMemoryBarrier
{
	VkPipelineStageFlags src_stage_mask{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};

	VkPipelineStageFlags dst_stage_mask{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

	VkAccessFlags src_access_mask{0};

	VkAccessFlags dst_access_mask{0};

	VkImageLayout old_layout{VK_IMAGE_LAYOUT_UNDEFINED};

	VkImageLayout new_layout{VK_IMAGE_LAYOUT_UNDEFINED};
};

/**
* @brief Buffer memory barrier structure used to define
*        memory access for a buffer during command recording.
*/
struct BufferMemoryBarrier
{
	VkPipelineStageFlags src_stage_mask{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};

	VkPipelineStageFlags dst_stage_mask{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

	VkAccessFlags src_access_mask{0};

	VkAccessFlags dst_access_mask{0};
};

/**
 * @brief Load and store info for a render pass attachment.
 */
struct LoadStoreInfo
{
	VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;

	VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
};

namespace gbuffer
{
/**
  * @return Load store info to load all and store only the swapchain
  */
std::vector<LoadStoreInfo> get_load_all_store_swapchain();

/**
  * @return Load store info to clear all and store only the swapchain
  */
std::vector<LoadStoreInfo> get_clear_all_store_swapchain();

/**
  * @return Load store info to clear and store all images
  */
std::vector<LoadStoreInfo> get_clear_store_all();

/**
  * @return Default clear values for the G-buffer
  */
std::vector<VkClearValue> get_clear_value();
}        // namespace gbuffer

}        // namespace vkb
