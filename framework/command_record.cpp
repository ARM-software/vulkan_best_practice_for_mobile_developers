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

#include "command_record.h"

#include "common/error.h"
#include "common/logging.h"
#include "core/descriptor_set_layout.h"
#include "core/device.h"
#include "core/shader_module.h"
#include "rendering/render_context.h"

namespace vkb
{
CommandRecord::CommandRecord(Device &device) :
    device{device}
{}

void CommandRecord::reset()
{
	// Clear stream content
	stream.str("");

	pipeline_state.reset();
	resource_binding_state.reset();
	descriptor_set_layout_state.clear();

	render_pass_bindings.clear();
	descriptor_set_bindings.clear();
	pipeline_bindings.clear();
}

Device &CommandRecord::get_device()
{
	return device;
}

const std::ostringstream &CommandRecord::get_stream() const
{
	return stream;
}

const std::vector<RenderPassBinding> &CommandRecord::get_render_pass_bindings() const
{
	return render_pass_bindings;
}

const std::vector<PipelineBinding> &CommandRecord::get_pipeline_bindings() const
{
	return pipeline_bindings;
}

const std::vector<DescriptorSetBinding> &CommandRecord::get_descriptor_set_bindings() const
{
	return descriptor_set_bindings;
}

void CommandRecord::begin(VkCommandBufferUsageFlags flags)
{
	// Write command parameters
	write(stream, CommandType::Begin, flags);
}

void CommandRecord::end()
{
	// Write command parameters
	write(stream, CommandType::End);
}

void vkb::CommandRecord::begin_render_pass(const RenderTarget &              render_target,
                                           const std::vector<LoadStoreInfo> &load_store_infos,
                                           const std::vector<VkClearValue> & clear_values)
{
	// Reset pipeline state
	pipeline_state.reset();
	resource_binding_state.reset();
	descriptor_set_layout_state.clear();

	RenderPassBinding render_pass_binding{stream.tellp(), render_target};
	render_pass_binding.load_store_infos = load_store_infos;
	render_pass_binding.clear_values     = clear_values;

	// Add first subpass to render pass
	auto &subpass              = render_pass_binding.subpasses.emplace_back(SubpassDesc{stream.tellp()});
	subpass.input_attachments  = render_target.get_input_attachments();
	subpass.output_attachments = render_target.get_output_attachments();

	// Update blend state attachments
	auto blend_state = pipeline_state.get_color_blend_state();
	blend_state.attachments.resize(subpass.output_attachments.size());
	pipeline_state.set_color_blend_state(blend_state);

	// Add render pass
	render_pass_bindings.push_back(render_pass_binding);
}

void CommandRecord::next_subpass()
{
	// Increment subpass index
	pipeline_state.set_subpass_index(pipeline_state.get_subpass_index() + 1);

	// Add subpass to render pass
	auto &render_pass_desc     = render_pass_bindings.back();
	auto &subpass              = render_pass_desc.subpasses.emplace_back(SubpassDesc{stream.tellp()});
	subpass.input_attachments  = render_pass_desc.render_target.get_input_attachments();
	subpass.output_attachments = render_pass_desc.render_target.get_output_attachments();

	// Update blend state attachments
	auto blend_state = pipeline_state.get_color_blend_state();
	blend_state.attachments.resize(subpass.output_attachments.size());
	pipeline_state.set_color_blend_state(blend_state);

	// Descriptor set
	descriptor_set_layout_state.clear();
	resource_binding_state.reset();

	// Write command parameters
	write(stream, CommandType::NextSubpass);
}

void CommandRecord::end_render_pass()
{
	auto &render_pass_desc = render_pass_bindings.back();

	std::vector<SubpassInfo> subpasses(render_pass_desc.subpasses.size());

	auto subpass_it = render_pass_desc.subpasses.begin();

	for (SubpassInfo &subpass : subpasses)
	{
		subpass.input_attachments  = subpass_it->input_attachments;
		subpass.output_attachments = subpass_it->output_attachments;

		++subpass_it;
	}

	render_pass_desc.render_pass = &device.get_resource_cache().request_render_pass(render_pass_desc.render_target.get_attachments(), render_pass_desc.load_store_infos, subpasses);
	render_pass_desc.framebuffer = &device.get_resource_cache().request_framebuffer(render_pass_desc.render_target, *render_pass_desc.render_pass);

	// Iterate over each graphics state that was bound within the subpass
	for (SubpassDesc &subpass_desc : render_pass_desc.subpasses)
	{
		for (auto &pipeline_desc : subpass_desc.pipeline_descs)
		{
			pipeline_desc.pipeline_state.set_render_pass(*render_pass_desc.render_pass);

			auto &pipeline = device.get_resource_cache().request_graphics_pipeline(pipeline_desc.pipeline_state);

			pipeline_bindings.push_back({pipeline_desc.event_id, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline});
		}
	}

	// Write command parameters
	write(stream, CommandType::EndRenderPass);
}

void CommandRecord::bind_pipeline_layout(PipelineLayout &pipeline_layout)
{
	pipeline_state.set_pipeline_layout(pipeline_layout);
}

void CommandRecord::set_specialization_constant(uint32_t constant_id, const std::vector<uint8_t> &data)
{
	pipeline_state.set_specialization_constant(constant_id, data);
}

void CommandRecord::push_constants(uint32_t offset, const std::vector<uint8_t> &values)
{
	const PipelineLayout &pipeline_layout = pipeline_state.get_pipeline_layout();

	VkShaderStageFlags shader_stage = pipeline_layout.get_push_constant_range_stage(offset, to_u32(values.size()));

	if (shader_stage)
	{
		// Write command parameters
		write(stream, CommandType::PushConstants, pipeline_layout.get_handle(), shader_stage, offset, values);
	}
	else
	{
		LOGW("Push constant range [{}, {}] not found", offset, values.size());
	}
}

void CommandRecord::bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element)
{
	resource_binding_state.bind_buffer(buffer, offset, range, set, binding, array_element);
}

void CommandRecord::bind_image(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element)
{
	resource_binding_state.bind_image(image_view, sampler, set, binding, array_element);
}

void CommandRecord::bind_input(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element)
{
	resource_binding_state.bind_input(image_view, set, binding, array_element);
}

void CommandRecord::bind_vertex_buffers(uint32_t first_binding, const std::vector<std::reference_wrapper<const vkb::core::Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets)
{
	std::vector<VkBuffer> native_buffers(buffers.size(), VK_NULL_HANDLE);
	std::transform(buffers.begin(), buffers.end(), native_buffers.begin(),
	               [](const core::Buffer &buffer) { return buffer.get_handle(); });
	// Write command parameters
	write(stream, CommandType::BindVertexBuffers, first_binding, native_buffers, offsets);
}

void CommandRecord::bind_index_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type)
{
	// Write command parameters
	write(stream, CommandType::BindIndexBuffer, buffer.get_handle(), offset, index_type);
}

void CommandRecord::set_viewport_state(const ViewportState &state_info)
{
	pipeline_state.set_viewport_state(state_info);
}

void CommandRecord::set_vertex_input_state(const VertexInputState &state_info)
{
	pipeline_state.set_vertex_input_state(state_info);
}

void CommandRecord::set_input_assembly_state(const InputAssemblyState &state_info)
{
	pipeline_state.set_input_assembly_state(state_info);
}

void CommandRecord::set_rasterization_state(const RasterizationState &state_info)
{
	pipeline_state.set_rasterization_state(state_info);
}

void CommandRecord::set_multisample_state(const MultisampleState &state_info)
{
	pipeline_state.set_multisample_state(state_info);
}

void CommandRecord::set_depth_stencil_state(const DepthStencilState &state_info)
{
	pipeline_state.set_depth_stencil_state(state_info);
}

void CommandRecord::set_color_blend_state(const ColorBlendState &state_info)
{
	pipeline_state.set_color_blend_state(state_info);
}

void CommandRecord::set_viewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports)
{
	// Write command parameters
	write(stream, CommandType::SetViewport, first_viewport, viewports);
}

void CommandRecord::set_scissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors)
{
	// Write command parameters
	write(stream, CommandType::SetScissor, first_scissor, scissors);
}

void CommandRecord::set_line_width(float line_width)
{
	// Write command parameters
	write(stream, CommandType::SetLineWidth, line_width);
}

void CommandRecord::set_depth_bias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor)
{
	// Write command parameters
	write(stream, CommandType::SetDepthBias, depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor);
}

void CommandRecord::set_blend_constants(const std::array<float, 4> &blend_constants)
{
	// Write command parameters
	write(stream, CommandType::SetBlendConstants, blend_constants);
}

void CommandRecord::set_depth_bounds(float min_depth_bounds, float max_depth_bounds)
{
	// Write command parameters
	write(stream, CommandType::SetDepthBounds, min_depth_bounds, max_depth_bounds);
}

void CommandRecord::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
	FlushPipelineState(VK_PIPELINE_BIND_POINT_GRAPHICS);

	FlushDescriptorState(VK_PIPELINE_BIND_POINT_GRAPHICS);

	// Write command parameters
	write(stream, CommandType::Draw, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandRecord::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{
	FlushPipelineState(VK_PIPELINE_BIND_POINT_GRAPHICS);

	FlushDescriptorState(VK_PIPELINE_BIND_POINT_GRAPHICS);

	// Write command parameters
	write(stream, CommandType::DrawIndexed, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandRecord::draw_indexed_indirect(const core::Buffer &buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride)
{
	FlushPipelineState(VK_PIPELINE_BIND_POINT_GRAPHICS);

	FlushDescriptorState(VK_PIPELINE_BIND_POINT_GRAPHICS);

	// Write command parameters
	write(stream, CommandType::DrawIndexedIndirect, buffer.get_handle(), offset, draw_count, stride);
}

void CommandRecord::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
	FlushPipelineState(VK_PIPELINE_BIND_POINT_COMPUTE);

	FlushDescriptorState(VK_PIPELINE_BIND_POINT_COMPUTE);

	// Write command parameters
	write(stream, CommandType::Dispatch, group_count_x, group_count_y, group_count_z);
}

void CommandRecord::dispatch_indirect(const core::Buffer &buffer, VkDeviceSize offset)
{
	FlushPipelineState(VK_PIPELINE_BIND_POINT_COMPUTE);

	FlushDescriptorState(VK_PIPELINE_BIND_POINT_COMPUTE);

	// Write command parameters
	write(stream, CommandType::DispatchIndirect, buffer.get_handle(), offset);
}

void CommandRecord::update_buffer(const core::Buffer &buffer, VkDeviceSize offset, const std::vector<uint8_t> &data)
{
	// Write command parameters
	write(stream, CommandType::UpdateBuffer, buffer.get_handle(), offset, data);
}

void CommandRecord::blit_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageBlit> &regions)
{
	// Write command parameters
	write(stream, CommandType::BlitImage, src_img.get_handle(), dst_img.get_handle(), regions);
}

void CommandRecord::copy_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageCopy> &regions)
{
	// Write command parameters
	write(stream, CommandType::CopyImage, src_img.get_handle(), dst_img.get_handle(), regions);
}

void CommandRecord::copy_buffer_to_image(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions)
{
	// Write command parameters
	write(stream, CommandType::CopyBufferToImage, buffer.get_handle(), image.get_handle(), regions);
}

void CommandRecord::image_memory_barrier(const core::ImageView &image_view, const ImageMemoryBarrier &memory_barrier)
{
	// Write command parameters
	write(stream, CommandType::ImageMemoryBarrier, image_view.get_image().get_handle(), image_view.get_subresource_range(), memory_barrier);
}

void CommandRecord::buffer_memory_barrier(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize size, const BufferMemoryBarrier &memory_barrier)
{
	// Write command parameters
	write(stream, CommandType::BufferMemoryBarrier, buffer.get_handle(), offset, size, memory_barrier);
}

void CommandRecord::FlushPipelineState(VkPipelineBindPoint pipeline_bind_point)
{
	// Create a new pipeline in the command stream only if the graphics state changed
	if (!pipeline_state.is_dirty())
	{
		return;
	}

	// Clear dirty bit for graphics state
	pipeline_state.clear_dirty();

	if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		const PipelineLayout &pipeline_layout = pipeline_state.get_pipeline_layout();

		SubpassDesc &subpass = render_pass_bindings.back().subpasses.back();

		// Add graphics state to the current subpass
		subpass.pipeline_descs.push_back({stream.tellp(), pipeline_state});
	}
	else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
	{
		auto &pipeline = device.get_resource_cache().request_compute_pipeline(pipeline_state);

		pipeline_bindings.push_back({stream.tellp(), pipeline_bind_point, pipeline});
	}
	else
	{
		throw "Only graphics and compute pipeline bind points are supported now";
	}
}

void CommandRecord::FlushDescriptorState(VkPipelineBindPoint pipeline_bind_point)
{
	PipelineLayout &pipeline_layout = const_cast<PipelineLayout &>(pipeline_state.get_pipeline_layout());

	const auto &set_bindings = pipeline_layout.get_bindings();

	std::unordered_set<uint32_t> update_sets;

	// Iterate over pipeline layout sets
	for (auto &set_it : set_bindings)
	{
		auto descriptor_set_layout_it = descriptor_set_layout_state.find(set_it.first);

		// Check if set was bound before
		if (descriptor_set_layout_it != descriptor_set_layout_state.end())
		{
			// Add set to later update it if is different from the current pipeline layout's set
			if (descriptor_set_layout_it->second->get_handle() != pipeline_layout.get_set_layout(set_it.first).get_handle())
			{
				update_sets.emplace(set_it.first);
			}
		}
	}

	// Remove bound descriptor set layouts which don't exists in the pipeline layout
	for (auto set_it = descriptor_set_layout_state.begin();
	     set_it != descriptor_set_layout_state.end();)
	{
		if (!pipeline_layout.has_set_layout(set_it->first))
		{
			set_it = descriptor_set_layout_state.erase(set_it);
		}
		else
		{
			++set_it;
		}
	}

	// Check if descriptor set needs to be created
	if (resource_binding_state.is_dirty() || !update_sets.empty())
	{
		// Clear dirty bit flag
		resource_binding_state.clear_dirty();

		// Iterate over all set bindings
		for (auto &set_it : resource_binding_state.get_set_bindings())
		{
			// Skip if set bindings don't have changes
			if (!set_it.second.is_dirty() && (update_sets.find(set_it.first) == update_sets.end()))
			{
				continue;
			}

			// Clear dirty flag for binding set
			resource_binding_state.clear_dirty(set_it.first);

			// Skip set layout if it doesn't exists
			if (!pipeline_layout.has_set_layout(set_it.first))
			{
				continue;
			}

			DescriptorSetLayout &descriptor_set_layout = pipeline_layout.get_set_layout(set_it.first);

			// Make descriptor set layout bound for current set
			descriptor_set_layout_state[set_it.first] = &descriptor_set_layout;

			BindingMap<VkDescriptorBufferInfo> buffer_infos;
			BindingMap<VkDescriptorImageInfo>  image_infos;

			std::vector<uint32_t> dynamic_offsets;

			// Iterate over all resource bindings
			for (auto &binding_it : set_it.second.get_resource_bindings())
			{
				auto  binding_index     = binding_it.first;
				auto &binding_resources = binding_it.second;

				VkDescriptorSetLayoutBinding binding_info;

				// Check if binding exists in the pipeline layout
				if (!descriptor_set_layout.get_layout_binding(binding_index, binding_info))
				{
					continue;
				}

				// Iterate over all binding resources
				for (auto &element_it : binding_resources)
				{
					auto  arrayElement  = element_it.first;
					auto &resource_info = element_it.second;

					// Get buffer info
					if (resource_info.is_buffer() && is_buffer_descriptor_type(binding_info.descriptorType))
					{
						VkDescriptorBufferInfo buffer_info = resource_info.get_buffer_info();

						if (is_dynamic_buffer_descriptor_type(binding_info.descriptorType))
						{
							dynamic_offsets.push_back(to_u32(buffer_info.offset));

							buffer_info.offset = 0;
						}

						buffer_infos[binding_index][arrayElement] = buffer_info;
					}
					// Get image info
					else if (resource_info.is_image_only() || resource_info.is_sampler_only() || resource_info.is_image_sampler())
					{
						VkDescriptorImageInfo image_info = resource_info.get_image_info();

						if (resource_info.is_image_only() || resource_info.is_image_sampler())
						{
							const vkb::core::ImageView &image_view = resource_info.get_image_view();

							// Add iamge layout info based on descriptor type
							switch (binding_info.descriptorType)
							{
								case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
								case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
									if (is_depth_stencil_format(image_view.get_format()))
									{
										image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
									}
									else
									{
										image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
									}
									break;
								case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
									image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
									break;

								default:
									continue;
							}
						}

						image_infos[binding_index][arrayElement] = std::move(image_info);
					}
				}
			}

			auto &descriptor_set = device.get_resource_cache().request_descriptor_set(descriptor_set_layout, buffer_infos, image_infos);

			descriptor_set_bindings.push_back({stream.tellp(), pipeline_bind_point, pipeline_layout, set_it.first, descriptor_set, dynamic_offsets});
		}
	}
}
}        // namespace vkb
