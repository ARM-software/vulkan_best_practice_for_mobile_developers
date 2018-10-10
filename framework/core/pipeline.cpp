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

#include "pipeline.h"

#include "device.h"
#include "pipeline_layout.h"
#include "shader_module.h"

namespace vkb
{
Pipeline::Pipeline(Device &device) :
    device{device}
{}

Pipeline::Pipeline(Pipeline &&other) :
    device{other.device},
    handle{other.handle}
{
	other.handle = VK_NULL_HANDLE;
}

Pipeline::~Pipeline()
{
	// Destroy pipeline
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device.get_handle(), handle, nullptr);
	}
}

VkPipeline Pipeline::get_handle() const
{
	return handle;
}

ComputePipeline::ComputePipeline(Device &                  device,
                                 const PipelineLayout &    pipeline_layout,
                                 const SpecializationInfo &specialization_info) :
    Pipeline{device}
{
	const ShaderModule &shader_module = pipeline_layout.get_stages().front();

	if (shader_module.get_stage() != VK_SHADER_STAGE_COMPUTE_BIT)
	{
		throw VulkanException{VK_ERROR_INVALID_SHADER_NV, "Shader module stage is not compute"};
	}

	VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

	stage.stage  = shader_module.get_stage();
	stage.module = shader_module.get_handle();
	stage.pName  = shader_module.get_entry_point().c_str();

	stage.pSpecializationInfo = &specialization_info.get_handle();

	VkComputePipelineCreateInfo create_info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

	create_info.layout = pipeline_layout.get_handle();
	create_info.stage  = stage;

	VkResult result = vkCreateComputePipelines(device.get_handle(), VK_NULL_HANDLE, 1, &create_info, 0, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create ComputePipelines"};
	}
}

GraphicsPipeline::GraphicsPipeline(Device &                                  device,
                                   GraphicsPipelineState &                   graphics_state,
                                   const ShaderStageMap<SpecializationInfo> &specialization_infos) :
    Pipeline{device}
{
	std::vector<VkPipelineShaderStageCreateInfo> stage_create_infos;

	for (const ShaderModule &shader_module : graphics_state.get_pipeline_layout().get_stages())
	{
		VkPipelineShaderStageCreateInfo stage_create_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

		stage_create_info.stage  = shader_module.get_stage();
		stage_create_info.module = shader_module.get_handle();
		stage_create_info.pName  = shader_module.get_entry_point().c_str();

		// Find if shader stage has specialization constants
		auto it = specialization_infos.find(stage_create_info.stage);

		if (it != specialization_infos.end())
		{
			stage_create_info.pSpecializationInfo = &it->second.get_handle();
		}

		stage_create_infos.push_back(stage_create_info);
	}

	VkGraphicsPipelineCreateInfo create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

	create_info.stageCount = to_u32(stage_create_infos.size());
	create_info.pStages    = stage_create_infos.data();

	VkPipelineVertexInputStateCreateInfo vertex_input_state{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	vertex_input_state.pVertexAttributeDescriptions    = graphics_state.get_vertex_input_state().attributes.data();
	vertex_input_state.vertexAttributeDescriptionCount = to_u32(graphics_state.get_vertex_input_state().attributes.size());

	vertex_input_state.pVertexBindingDescriptions    = graphics_state.get_vertex_input_state().bindings.data();
	vertex_input_state.vertexBindingDescriptionCount = to_u32(graphics_state.get_vertex_input_state().bindings.size());

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

	input_assembly_state.topology               = graphics_state.get_input_assembly_state().topology;
	input_assembly_state.primitiveRestartEnable = graphics_state.get_input_assembly_state().primitive_restart_enable;

	VkPipelineViewportStateCreateInfo viewport_state{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

	viewport_state.viewportCount = graphics_state.get_viewport_state().viewport_count;
	viewport_state.scissorCount  = graphics_state.get_viewport_state().scissor_count;

	VkPipelineRasterizationStateCreateInfo rasterization_state{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

	rasterization_state.depthClampEnable        = graphics_state.get_rasterization_state().depth_clamp_enable;
	rasterization_state.rasterizerDiscardEnable = graphics_state.get_rasterization_state().rasterizer_discard_enable;
	rasterization_state.polygonMode             = graphics_state.get_rasterization_state().polygon_mode;
	rasterization_state.cullMode                = graphics_state.get_rasterization_state().cull_mode;
	rasterization_state.frontFace               = graphics_state.get_rasterization_state().front_face;
	rasterization_state.depthBiasEnable         = graphics_state.get_rasterization_state().depth_bias_enable;
	rasterization_state.depthBiasClamp          = 1.0f;
	rasterization_state.depthBiasSlopeFactor    = 1.0f;
	rasterization_state.lineWidth               = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample_state{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

	multisample_state.sampleShadingEnable   = graphics_state.get_multisample_state().sample_shading_enable;
	multisample_state.rasterizationSamples  = graphics_state.get_multisample_state().rasterization_samples;
	multisample_state.minSampleShading      = graphics_state.get_multisample_state().min_sample_shading;
	multisample_state.alphaToCoverageEnable = graphics_state.get_multisample_state().alpha_to_coverage_enable;
	multisample_state.alphaToOneEnable      = graphics_state.get_multisample_state().alpha_to_one_enable;

	if (graphics_state.get_multisample_state().sample_mask)
	{
		multisample_state.pSampleMask = &graphics_state.get_multisample_state().sample_mask;
	}

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

	depth_stencil_state.depthTestEnable       = graphics_state.get_depth_stencil_state().depth_test_enable;
	depth_stencil_state.depthWriteEnable      = graphics_state.get_depth_stencil_state().depth_write_enable;
	depth_stencil_state.depthCompareOp        = graphics_state.get_depth_stencil_state().depth_compare_op;
	depth_stencil_state.depthBoundsTestEnable = graphics_state.get_depth_stencil_state().depth_bounds_test_enable;
	depth_stencil_state.stencilTestEnable     = graphics_state.get_depth_stencil_state().stencil_test_enable;
	depth_stencil_state.front.failOp          = graphics_state.get_depth_stencil_state().front.fail_op;
	depth_stencil_state.front.passOp          = graphics_state.get_depth_stencil_state().front.pass_op;
	depth_stencil_state.front.depthFailOp     = graphics_state.get_depth_stencil_state().front.depth_fail_op;
	depth_stencil_state.front.compareOp       = graphics_state.get_depth_stencil_state().front.compare_op;
	depth_stencil_state.front.compareMask     = ~0U;
	depth_stencil_state.front.writeMask       = ~0U;
	depth_stencil_state.front.reference       = ~0U;
	depth_stencil_state.back.failOp           = graphics_state.get_depth_stencil_state().back.fail_op;
	depth_stencil_state.back.passOp           = graphics_state.get_depth_stencil_state().back.pass_op;
	depth_stencil_state.back.depthFailOp      = graphics_state.get_depth_stencil_state().back.depth_fail_op;
	depth_stencil_state.back.compareOp        = graphics_state.get_depth_stencil_state().back.compare_op;
	depth_stencil_state.back.compareMask      = ~0U;
	depth_stencil_state.back.writeMask        = ~0U;
	depth_stencil_state.back.reference        = ~0U;

	VkPipelineColorBlendStateCreateInfo color_blend_state{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

	color_blend_state.logicOpEnable     = graphics_state.get_color_blend_state().logic_op_enable;
	color_blend_state.logicOp           = graphics_state.get_color_blend_state().logic_op;
	color_blend_state.attachmentCount   = to_u32(graphics_state.get_color_blend_state().attachments.size());
	color_blend_state.pAttachments      = reinterpret_cast<const VkPipelineColorBlendAttachmentState *>(graphics_state.get_color_blend_state().attachments.data());
	color_blend_state.blendConstants[0] = 1.0f;
	color_blend_state.blendConstants[1] = 1.0f;
	color_blend_state.blendConstants[2] = 1.0f;
	color_blend_state.blendConstants[3] = 1.0f;

	std::array<VkDynamicState, 9> dynamic_states{
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR,
	    VK_DYNAMIC_STATE_LINE_WIDTH,
	    VK_DYNAMIC_STATE_DEPTH_BIAS,
	    VK_DYNAMIC_STATE_BLEND_CONSTANTS,
	    VK_DYNAMIC_STATE_DEPTH_BOUNDS,
	    VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
	    VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
	    VK_DYNAMIC_STATE_STENCIL_REFERENCE,
	};

	VkPipelineDynamicStateCreateInfo dynamic_state{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

	dynamic_state.pDynamicStates    = dynamic_states.data();
	dynamic_state.dynamicStateCount = to_u32(dynamic_states.size());

	create_info.pVertexInputState   = &vertex_input_state;
	create_info.pInputAssemblyState = &input_assembly_state;
	create_info.pViewportState      = &viewport_state;
	create_info.pRasterizationState = &rasterization_state;
	create_info.pMultisampleState   = &multisample_state;
	create_info.pDepthStencilState  = &depth_stencil_state;
	create_info.pColorBlendState    = &color_blend_state;
	create_info.pDynamicState       = &dynamic_state;

	create_info.layout     = graphics_state.get_pipeline_layout().get_handle();
	create_info.renderPass = graphics_state.get_render_pass().get_handle();
	create_info.subpass    = graphics_state.get_subpass_index();

	auto result = vkCreateGraphicsPipelines(device.get_handle(), VK_NULL_HANDLE, 1, &create_info, NULL, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create GraphicsPipelines"};
	}
}
}        // namespace vkb
