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

#include "common.h"
#include "graphics_pipeline_state.h"

namespace vkb
{
class Device;
class PipelineLayout;
class ShaderModule;
class SpecializationInfo;

class Pipeline : public NonCopyable
{
  public:
	Pipeline(Device &device);

	Pipeline(Pipeline &&other);

	virtual ~Pipeline();

	void destroy();

	VkPipeline get_handle() const;

  protected:
	Device &device;

	VkPipeline handle = VK_NULL_HANDLE;
};

class ComputePipeline : public Pipeline
{
  public:
	ComputePipeline(ComputePipeline &&) = default;

	virtual ~ComputePipeline() = default;

	ComputePipeline(Device &                  device,
	                const PipelineLayout &    pipeline_layout,
	                const SpecializationInfo &specialization_info);
};

class GraphicsPipeline : public Pipeline
{
  public:
	GraphicsPipeline(GraphicsPipeline &&) = default;

	virtual ~GraphicsPipeline() = default;

	GraphicsPipeline(Device &                                  device,
	                 GraphicsPipelineState &                   graphics_state,
	                 const ShaderStageMap<SpecializationInfo> &specialization_infos);
};
}        // namespace vkb
