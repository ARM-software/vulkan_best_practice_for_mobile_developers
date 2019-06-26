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

#include "utils.h"

#include "core/pipeline_layout.h"
#include "core/shader_module.h"

#include "scene_graph/components/image.h"
#include "scene_graph/components/material.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/pbr_material.h"
#include "scene_graph/components/sampler.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"

#include <queue>
#include <stdexcept>

namespace vkb
{
std::string get_extension(const std::string &uri)
{
	auto dot_pos = uri.find_last_of('.');
	if (dot_pos == std::string::npos)
	{
		throw std::runtime_error{"Uri has no extension"};
	}

	return uri.substr(dot_pos + 1);
}

void screenshot(RenderContext &render_context, const std::string &filename)
{
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	// We want the last completed frame since we don't want to be reading from an incomplete swapchain image
	auto &frame          = render_context.get_last_rendered_frame();
	auto &src_image_view = frame.get_render_target().get_views().at(0);

	bool blit_supported = true;

	// Check if device supports blitting from swapchain images to linear images
	{
		auto format_properties = render_context.get_device().get_format_properties(render_context.get_swapchain().get_format());

		if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
		{
			blit_supported = false;
		}

		format_properties = render_context.get_device().get_format_properties(format);
		if (!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
		{
			blit_supported = false;
		}
	}

	auto width  = render_context.get_swapchain().get_extent().width;
	auto height = render_context.get_swapchain().get_extent().height;

	core::Image dst_image{render_context.get_device(),
	                      VkExtent3D{width, height, 1},
	                      format,
	                      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	                      VMA_MEMORY_USAGE_CPU_ONLY,
	                      VK_SAMPLE_COUNT_1_BIT,
	                      1, 1,
	                      VK_IMAGE_TILING_LINEAR};

	ImageView dst_image_view{dst_image, VK_IMAGE_VIEW_TYPE_2D};

	const auto &queue = render_context.get_device().get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);

	auto &cmd_buf = render_context.get_device().request_command_buffer();

	cmd_buf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// Enable destination image to be written to
	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.src_access_mask = 0;
		memory_barrier.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

		cmd_buf.image_memory_barrier(dst_image_view, memory_barrier);
	}

	// Enable swapchain image to be read from
	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		memory_barrier.new_layout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		cmd_buf.image_memory_barrier(src_image_view, memory_barrier);
	}

	bool swizzle = false;

	if (blit_supported)
	{
		// Blit whole swapchain image (does automatic format conversion)
		VkImageBlit image_blit_region{};
		image_blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_blit_region.srcSubresource.layerCount = 1;
		image_blit_region.srcOffsets[1]             = {static_cast<int32_t>(width), static_cast<int32_t>(height), 1};
		image_blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_blit_region.dstSubresource.layerCount = 1;
		image_blit_region.dstOffsets[1]             = {static_cast<int32_t>(width), static_cast<int32_t>(height), 1};

		cmd_buf.blit_image(src_image_view.get_image(), dst_image, {image_blit_region});
	}
	else
	{
		LOGW("Device does not support blitting of images, using a copy instead");

		// Check if swapchain images are in a BGR format
		auto bgr_formats = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
		swizzle          = std::find(bgr_formats.begin(), bgr_formats.end(), render_context.get_swapchain().get_format()) != bgr_formats.end();

		// Copy whole swapchain image
		VkImageCopy image_copy_region{};
		image_copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_copy_region.srcSubresource.layerCount = 1;
		image_copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_copy_region.dstSubresource.layerCount = 1;
		image_copy_region.extent.width              = width;
		image_copy_region.extent.height             = height;
		image_copy_region.extent.depth              = 1;

		cmd_buf.copy_image(src_image_view.get_image(), dst_image, {image_copy_region});
	}

	// Enable destination image to map image memory
	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memory_barrier.dst_access_mask = VK_ACCESS_MEMORY_READ_BIT;
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_GENERAL;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

		cmd_buf.image_memory_barrier(dst_image_view, memory_barrier);
	}

	// Revert back the swapchain image from transfer to present
	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		memory_barrier.new_layout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		cmd_buf.image_memory_barrier(src_image_view, memory_barrier);
	}

	cmd_buf.end();

	queue.submit(cmd_buf, frame.get_fence_pool().request_fence());

	queue.wait_idle();

	auto raw_data = dst_image.map();

	// Android requires the sub resource to be queried while the memory is mapped
	VkImageSubresource  subresource{VK_IMAGE_ASPECT_COLOR_BIT};
	VkSubresourceLayout subresource_layout;
	vkGetImageSubresourceLayout(render_context.get_device().get_handle(), dst_image.get_handle(), &subresource, &subresource_layout);

	auto image_data = std::vector<uint8_t>(width * height * 3);

	// Read in only RGB data
	size_t j = 0;
	for (size_t i = 0; i < width * height * 4; i += 4)
	{
		image_data[j]     = raw_data[i];
		image_data[j + 1] = raw_data[i + 1];
		image_data[j + 2] = raw_data[i + 2];

		// Switch B and R components of each pixel if swapchain image format isn't RGB
		if (swizzle)
		{
			std::swap(image_data[j], image_data[j + 2]);
		}

		j += 3;
	}

	dst_image.unmap();

	vkb::file::write_image(image_data,
	                       filename,
	                       width,
	                       height,
	                       3,
	                       static_cast<uint32_t>(subresource_layout.rowPitch) * 3 / 4);
}

namespace
{
VkShaderStageFlagBits find_shader_stage(const std::string &ext)
{
	if (ext == "vert")
	{
		return VK_SHADER_STAGE_VERTEX_BIT;
	}
	else if (ext == "frag")
	{
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	else if (ext == "comp")
	{
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
	else if (ext == "geom")
	{
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	}
	else if (ext == "tesc")
	{
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	}
	else if (ext == "tese")
	{
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}

	throw std::runtime_error("File extension `" + ext + "` does not have a vulkan shader stage.");
};
}        // namespace

glm::mat4 vulkan_style_projection(const glm::mat4 &proj)
{
	// Flip Y in clipspace. X = -1, Y = -1 is topLeft in Vulkan.
	glm::mat4 mat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

	// Z depth is [0, 1] range instead of [-1, 1].
	mat = glm::scale(mat, glm::vec3(1.0f, 1.0f, 0.5f));

	return glm::translate(mat, glm::vec3(0.0f, 0.0f, 1.0f)) * proj;
}

std::string to_snake_case(const std::string &text)
{
	std::stringstream result;

	for (const auto ch : text)
	{
		if (std::isalpha(ch))
		{
			if (std::isspace(ch))
			{
				result << "_";
			}
			else
			{
				if (std::isupper(ch))
				{
					result << "_";
				}

				result << static_cast<char>(std::tolower(ch));
			}
		}
		else
		{
			result << ch;
		}
	}

	return result.str();
}
}        // namespace vkb
