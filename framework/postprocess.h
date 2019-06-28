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

#include <map>
#include <vector>

#include "common/vk_common.h"
#include "core/Image.h"
#include "framework.h"

namespace vkb
{
/// @brief The description of a postprocess pass
struct PostProcessPass
{
	/// The fragment shader to be used in the pass
	std::string fragment_shader_path;

	/// The output images for the pass, described with their format
	std::vector<VkFormat> image_formats = {};

	/// Relative Width of the output image(s) with respect to the swapchain
	float relative_width = 1.0f;

	/// Relative height of the output image(s) with respect to the swapchain
	float relative_height = 1.0f;
};

/// @brief The set of input images for the postprocess chain
typedef std::vector<core::Image> ChainInputImages;

/// @brief Helper class to manage Vulkan objects related to postprocessing
class PostProcess
{
  public:
	/// @brief Constructs a PostProcess object
	PostProcess(Context *context);

	/// @brief Destroys the PostProcess object
	~PostProcess();

	/// @brief Tears down Vulkan objects used in postprocessing
	void teardown_postprocess();

	/// @brief Initializes the postprocess chain
	/// @param input_images A vector containing the input images to be used for postprocessing.
	/// The vector indices correspond to frame indices, the entries are vectors of input images
	/// for the first pass in the chain. Input images must be transitioned to SHADER_READ_ONLY_OPTIMAL
	/// layout before the first postprocess pass starts.
	/// @param passes A description of the postprocess passes. The last pass will be used to draw to
	/// the swapchain image, so it must have just one output.
	/// @returns true if initialization succeeded, false otherwise
	bool init(std::vector<ChainInputImages> input_images, std::vector<PostProcessPass> passes);

	/// @brief Re-initializes framebuffers after the swapchain is recreated, passing the new set
	/// of input images
	/// @param input_images A vector containing the input images to be used for postprocessing.
	/// The vector indices correspond to frame indices, the entries are vectors of input images
	/// for the first pass in the chain. Input images must be transitioned to SHADER_READ_ONLY_OPTIMAL
	/// layout before the first postprocess pass starts.
	void on_resize(std::vector<ChainInputImages> input_images);

	/// @brief Prepares and submits command buffers for postprocessing
	/// @param swapchain_index The swapchain index for the image being rendered
	void run(uint32_t swapchain_index);

  private:
	Context *context = nullptr;

	/// @brief The Vulkan objects needed to run a postprocess pass
	struct PostProcessPassResources
	{
		/// The postprocess pass description
		PostProcessPass desc = {};

		/// The images to be used in the pass.
		std::vector<Image> images = {};

		/// The descriptor set layout for this pass
		VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;

		/// The descriptor set holding the input images
		VkDescriptorSet desc_set = VK_NULL_HANDLE;

		/// The framebuffer to be used in the pass.
		VkFramebuffer framebuffer = VK_NULL_HANDLE;

		/// The render pass to be used in the pass.
		VkRenderPass render_pass = VK_NULL_HANDLE;

		/// The pipeline layout to be used in the pass.
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

		/// The pipeline to be used in the pass.
		VkPipeline pipeline = VK_NULL_HANDLE;
	};

	/// @brief The Vulkan objects needed to run the postprocess chain
	typedef std::vector<PostProcessPassResources> PostProcessChainResources;

	/// @brief The Vulkan objects needed to run postprocessing on each swapchain image
	typedef std::vector<PostProcessChainResources> PostProcessResources;

	std::vector<PostProcessPass>  passes;
	std::vector<ChainInputImages> input_images;
	PostProcessResources          pp_resources;
	VkDescriptorPool              desc_pool = VK_NULL_HANDLE;
	VkSampler                     sampler   = VK_NULL_HANDLE;
};

}        // namespace vkb
