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

#include "gui.h"
#include "imgui_internal.h"

#include <map>
#include <numeric>

#include "render_context.h"
#include "utils.h"

#include "core/descriptor_set.h"
#include "core/descriptor_set_layout.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/shader_module.h"
#include "vulkan_sample.h"

namespace vkb
{
namespace
{
inline void reset_graph_max_value(Gui::StatsView::GraphData &graph_data)
{
	// If it does not have a fixed max
	if (!graph_data.has_fixed_max)
	{
		// Reset it
		graph_data.max_value = 0.0f;
	}
}
}        // namespace

const float Gui::press_time_ms = 200.0f;

const float Gui::font_size_dp = 21.0f;

const float Gui::overlay_alpha = 0.3f;

const ImGuiWindowFlags Gui::common_flags = ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoScrollbar |
                                           ImGuiWindowFlags_NoTitleBar |
                                           ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_AlwaysAutoResize |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoFocusOnAppearing |
                                           ImGuiWindowFlags_NoNav;

const ImGuiWindowFlags Gui::options_flags = Gui::common_flags;

const ImGuiWindowFlags Gui::info_flags = Gui::common_flags | ImGuiWindowFlags_NoInputs;

Gui::Gui(RenderContext &render_context, const float dpi_factor) :
    render_context{render_context},
    dpi_factor{dpi_factor},
    pipeline_layout{create_pipeline_layout(render_context.get_device(), "shaders/imgui.vert", "shaders/imgui.frag")}
{
	ImGui::CreateContext();

	ImGuiStyle &style = ImGui::GetStyle();

	// Color scheme
	style.Colors[ImGuiCol_TitleBg]       = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_CheckMark]     = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

	// Borderless window
	style.WindowBorderSize = 0.0f;

	// Global scale
	style.ScaleAllSizes(dpi_factor);

	// Dimensions
	ImGuiIO &io                = ImGui::GetIO();
	auto &   extent            = render_context.get_swapchain().get_extent();
	io.DisplaySize.x           = static_cast<float>(extent.width);
	io.DisplaySize.y           = static_cast<float>(extent.height);
	io.FontGlobalScale         = 1.0f;
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	// Roboto font
	const float size_pixels = font_size_dp * dpi_factor;
	io.Fonts->AddFontFromMemoryCompressedTTF(roboto_font.data, roboto_font.size, size_pixels);

	// Create font texture
	unsigned char *font_data;
	int            tex_width, tex_height;
	io.Fonts->GetTexDataAsRGBA32(&font_data, &tex_width, &tex_height);
	size_t upload_size = tex_width * tex_height * 4 * sizeof(char);

	auto &device = render_context.get_device();

	// Create target image for copy
	VkExtent3D font_extent{to_u32(tex_width), to_u32(tex_height), 1u};
	font_image      = std::make_unique<core::Image>(device, font_extent, VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                               VMA_MEMORY_USAGE_GPU_ONLY);
	font_image_view = std::make_unique<ImageView>(*font_image, VK_IMAGE_VIEW_TYPE_2D);

	// Upload font data into the vulkan image memory
	{
		core::Buffer stage_buffer{device, upload_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY};
		stage_buffer.update(0, {font_data, font_data + upload_size});

		auto &graphics_queue = device.get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);
		auto &command_buffer = device.request_command_buffer();

		FencePool fence_pool{device};

		// Begin recording
		command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		{
			// Prepare for transfer
			ImageMemoryBarrier memory_barrier{};
			memory_barrier.old_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
			memory_barrier.new_layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			memory_barrier.src_access_mask = 0;
			memory_barrier.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_HOST_BIT;
			memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

			command_buffer.image_memory_barrier(*font_image_view, memory_barrier);
		}

		// Copy
		VkBufferImageCopy buffer_copy_region{};
		buffer_copy_region.imageSubresource.layerCount = font_image_view->get_subresource_range().layerCount;
		buffer_copy_region.imageSubresource.aspectMask = font_image_view->get_subresource_range().aspectMask;
		buffer_copy_region.imageExtent                 = font_image->get_extent();

		command_buffer.copy_buffer_to_image(stage_buffer, *font_image, {buffer_copy_region});

		{
			// Prepare for fragmen shader
			ImageMemoryBarrier memory_barrier{};
			memory_barrier.old_layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			memory_barrier.new_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			memory_barrier.src_access_mask = 0;
			memory_barrier.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
			memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;
			memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			command_buffer.image_memory_barrier(*font_image_view, memory_barrier);
		}

		// End recording
		command_buffer.end();

		auto &queue = device.get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);

		queue.submit(command_buffer, device.request_fence());

		// Wait for the command buffer to finish its work before destroying the staging buffer
		device.get_fence_pool().wait();
		device.get_fence_pool().reset();
		device.get_command_pool().reset();
	}

	// Create texture sampler
	VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sampler_info.maxAnisotropy = 1.0f;
	sampler_info.magFilter     = VK_FILTER_LINEAR;
	sampler_info.minFilter     = VK_FILTER_LINEAR;
	sampler_info.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK(vkCreateSampler(device.get_handle(), &sampler_info, nullptr, &sampler));
}

void Gui::update(const float delta_time)
{
	if (!visible)
	{
		ImGui::EndFrame();
		return;
	}

	// Update imGui
	ImGuiIO &io  = ImGui::GetIO();
	io.DeltaTime = delta_time;

	// Render to generate draw buffers
	ImGui::Render();

	// Update vulkan buffers
	update_buffers();
}

void Gui::update_buffers()
{
	ImDrawData *draw_data = ImGui::GetDrawData();

	size_t vertex_buffer_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
	size_t index_buffer_size  = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

	if ((vertex_buffer_size == 0) || (index_buffer_size == 0))
	{
		return;
	}

	std::vector<uint8_t> vertex_data(vertex_buffer_size);
	std::vector<uint8_t> index_data(index_buffer_size);

	// Upload data
	ImDrawVert *vtx_dst = (ImDrawVert *) vertex_data.data();
	ImDrawIdx * idx_dst = (ImDrawIdx *) index_data.data();

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}

	auto &frame = render_context.get_active_frame();
	// Get vertex buffer for the current image
	auto &vertex_buffer = frame.gui_vertex_buffer;

	// Upload to buffers to make writes visible to GPU
	// If buffer size is not enough
	if (vertex_buffer && vertex_buffer->get_size() < vertex_buffer_size)
	{
		// Destroy the old buffer before creating a new one
		vertex_buffer.reset();
	}

	if (!vertex_buffer)
	{
		// Create a buffer for vertices
		vertex_buffer = std::make_unique<core::Buffer>(render_context.get_device(), vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	}

	// Update current buffer with new data
	vertex_buffer->update(0, vertex_data);

	// Get vertex buffer for the current image
	auto &index_buffer = frame.gui_index_buffer;

	// If buffer size is not enough
	if (index_buffer && index_buffer->get_size() < index_buffer_size)
	{
		// Destroy the old buffer before creating a new one
		index_buffer.reset();
	}

	if (!index_buffer)
	{
		// Create a buffer for indices
		index_buffer = std::make_unique<core::Buffer>(render_context.get_device(), index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	}

	// Update current buffer with new data
	index_buffer->update(0, index_data);
}

void Gui::resize(const uint32_t width, const uint32_t height) const
{
	auto &io         = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(width);
	io.DisplaySize.y = static_cast<float>(height);
}

void Gui::draw(CommandBuffer &command_buffer)
{
	if (!visible)
	{
		return;
	}

	// Vertex input state
	VkVertexInputBindingDescription vertex_input_binding{};
	vertex_input_binding.stride = to_u32(sizeof(ImDrawVert));

	// Location 0: Position
	VkVertexInputAttributeDescription pos_attr{};
	pos_attr.format = VK_FORMAT_R32G32_SFLOAT;
	pos_attr.offset = to_u32(offsetof(ImDrawVert, pos));

	// Location 1: UV
	VkVertexInputAttributeDescription uv_attr{};
	uv_attr.location = 1;
	uv_attr.format   = VK_FORMAT_R32G32_SFLOAT;
	uv_attr.offset   = to_u32(offsetof(ImDrawVert, uv));

	// Location 2: Color
	VkVertexInputAttributeDescription col_attr{};
	col_attr.location = 2;
	col_attr.format   = VK_FORMAT_R8G8B8A8_UNORM;
	col_attr.offset   = to_u32(offsetof(ImDrawVert, col));

	VertexInputState vertex_input_state{};
	vertex_input_state.bindings   = {vertex_input_binding};
	vertex_input_state.attributes = {pos_attr, uv_attr, col_attr};

	command_buffer.set_vertex_input_state(vertex_input_state);

	// Blend state
	vkb::ColorBlendAttachmentState color_attachment{};
	color_attachment.blend_enable           = VK_TRUE;
	color_attachment.color_write_mask       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
	color_attachment.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_attachment.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_attachment.src_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	vkb::ColorBlendState blend_state{};
	blend_state.attachments = {color_attachment};

	command_buffer.set_color_blend_state(blend_state);

	vkb::RasterizationState rasterization_state{};
	rasterization_state.cull_mode = VK_CULL_MODE_NONE;
	command_buffer.set_rasterization_state(rasterization_state);

	vkb::DepthStencilState depth_state{};
	depth_state.depth_test_enable  = VK_FALSE;
	depth_state.depth_write_enable = VK_FALSE;
	command_buffer.set_depth_stencil_state(depth_state);

	// Bind pipeline layout
	command_buffer.bind_pipeline_layout(pipeline_layout);

	command_buffer.bind_image(*font_image_view, sampler, 0, 0, 0);

	// Pre-rotation
	auto      transform      = render_context.get_swapchain().get_transform();
	auto &    io             = ImGui::GetIO();
	auto      push_transform = glm::mat4(1.0f);
	glm::vec3 rotation_axis  = glm::vec3(0.0f, 0.0f, 1.0f);
	if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
	{
		push_transform = glm::rotate(push_transform, glm::radians(90.0f), rotation_axis);
	}
	else if (transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
	{
		push_transform = glm::rotate(push_transform, glm::radians(270.0f), rotation_axis);
	}
	else if (transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
	{
		push_transform = glm::rotate(push_transform, glm::radians(180.0f), rotation_axis);
	}
	// GUI coordinate space to screen space
	push_transform = glm::translate(push_transform, glm::vec3(-1.0f, -1.0f, 0.0f));
	push_transform = glm::scale(push_transform, glm::vec3(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y, 0.0f));

	// Push constants
	command_buffer.push_constants(0, push_transform);

	// Render commands
	ImDrawData *draw_data     = ImGui::GetDrawData();
	int32_t     vertex_offset = 0;
	uint32_t    index_offset  = 0;

	if (draw_data->CmdListsCount > 0)
	{
		auto &frame = render_context.get_active_frame();

		// Vertex buffers
		assert(frame.gui_vertex_buffer && "Gui vertex buffer is invalid");
		const auto &vertex_buffer = *frame.gui_vertex_buffer;

		std::vector<std::reference_wrapper<const core::Buffer>> buffers;
		buffers.emplace_back(std::ref(vertex_buffer));

		std::vector<VkDeviceSize> offsets{0};

		command_buffer.bind_vertex_buffers(0, buffers, offsets);

		// Index buffer
		assert(frame.gui_index_buffer && "Gui index buffer is invalid");
		const auto &index_buffer = *frame.gui_index_buffer;
		command_buffer.bind_index_buffer(index_buffer, 0, VK_INDEX_TYPE_UINT16);

		for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
		{
			const ImDrawList *cmd_list = draw_data->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd *cmd = &cmd_list->CmdBuffer[j];
				VkRect2D         scissor_rect;
				// Adjust for pre-rotation if necessary
				if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
				{
					scissor_rect.offset.x      = static_cast<uint32_t>(io.DisplaySize.y - cmd->ClipRect.w);
					scissor_rect.offset.y      = static_cast<uint32_t>(cmd->ClipRect.x);
					scissor_rect.extent.width  = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
					scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
				}
				else if (transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
				{
					scissor_rect.offset.x      = static_cast<uint32_t>(cmd->ClipRect.y);
					scissor_rect.offset.y      = static_cast<uint32_t>(io.DisplaySize.x - cmd->ClipRect.z);
					scissor_rect.extent.width  = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
					scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
				}
				else if (transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
				{
					scissor_rect.offset.x      = static_cast<uint32_t>(io.DisplaySize.x - cmd->ClipRect.z);
					scissor_rect.offset.y      = static_cast<uint32_t>(io.DisplaySize.y - cmd->ClipRect.w);
					scissor_rect.extent.width  = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
					scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
				}
				else
				{
					scissor_rect.offset.x      = std::max(static_cast<int32_t>(cmd->ClipRect.x), 0);
					scissor_rect.offset.y      = std::max(static_cast<int32_t>(cmd->ClipRect.y), 0);
					scissor_rect.extent.width  = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
					scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
				}

				command_buffer.set_scissor(0, {scissor_rect});
				command_buffer.draw_indexed(cmd->ElemCount, 1, index_offset, vertex_offset, 0);
				index_offset += cmd->ElemCount;
			}
			vertex_offset += cmd_list->VtxBuffer.Size;
		}
	}
}

Gui::~Gui()
{
	if (sampler)
	{
		vkDestroySampler(render_context.get_device().get_handle(), sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}

	ImGui::DestroyContext();
}

void Gui::show_demo_window()
{
	ImGui::ShowDemoWindow();
}

Gui::StatsView &Gui::get_stats_view()
{
	return stats_view;
}

void Gui::StatsView::reset_max_value(const StatIndex index)
{
	auto pr = graph_map.find(index);
	if (pr != graph_map.end())
	{
		reset_graph_max_value(pr->second);
	}
}

void Gui::StatsView::reset_max_values()
{
	// For every entry in the map
	std::for_each(graph_map.begin(),
	              graph_map.end(),
	              [](auto &pr) { reset_graph_max_value(pr.second); });
}

void Gui::show_top_window(const std::string &app_name, const Stats *stats, DebugInfo *debug_info)
{
	// Transparent background
	ImGui::SetNextWindowBgAlpha(overlay_alpha);
	ImVec2 size{ImGui::GetIO().DisplaySize.x, 0.0f};
	ImGui::SetNextWindowSize(size, ImGuiSetCond_Always);

	// Top left
	ImVec2 pos{0.0f, 0.0f};
	ImGui::SetNextWindowPos(pos, ImGuiSetCond_Always);

	bool is_open = true;
	ImGui::Begin("Top", &is_open, common_flags);

	show_app_info(app_name);

	if (stats)
	{
		show_stats(*stats);

		// Reset max values if user taps on this window
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0 /* left */))
		{
			stats_view.reset_max_values();
		}
	}

	if (debug_info)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(0.0f, 0.35f, 0.55f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(0.0f, 0.5f, 0.75f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(0.5f, 0.35f, 0.55f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, debug_view.round_corners);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, debug_view.frame_padding);
		ImGui::Dummy(ImVec2(0.0f, debug_view.button_padding));
		if (ImGui::Button(debug_view.active ? "Disable debug info" : "Enable debug info"))
		{
			debug_view.active = !debug_view.active;
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::Dummy(ImVec2(0.0f, debug_view.button_padding));

		if (debug_view.active)
		{
			show_debug_window(*debug_info, ImVec2{debug_view.button_padding, debug_view.button_padding + ImGui::GetWindowSize().y});
		}
	}

	ImGui::End();
}

void Gui::show_app_info(const std::string &app_name)
{
	// Sample name
	ImGui::Text("%s", app_name.c_str());

	// GPU name
	auto &device            = render_context.get_device();
	auto  device_name_label = "GPU: " + std::string(device.get_properties().deviceName);
	ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(device_name_label.c_str()).x);
	ImGui::Text("%s", device_name_label.c_str());
}

void Gui::show_debug_window(DebugInfo &debug_info, const ImVec2 &position)
{
	ImGui::SetNextWindowBgAlpha(overlay_alpha);
	ImGui::SetNextWindowPos(position, ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowContentSize(ImVec2{debug_view.window_size.x * dpi_factor, debug_view.window_size.y * dpi_factor});

	bool is_open = true;
	ImGui::Begin(debug_view.title, &is_open, common_flags);

	ImGui::Separator();
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, debug_view.column_width * dpi_factor);
	for (auto &field : debug_info.get_fields())
	{
		const std::string &label = field->label;
		const std::string &value = field->to_string();

		ImGui::Text("%s", label.c_str());
		ImGui::NextColumn();
		ImGui::Text(" %s", value.c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::Separator();

	ImGui::End();
}

Gui::StatsView::GraphData::GraphData(const std::string &graph_label_format_,
                                     float              scale_factor_,
                                     bool               has_fixed_max_,
                                     float              max_value_) :
    graph_label_format{graph_label_format_},
    scale_factor{scale_factor_},
    has_fixed_max{has_fixed_max_},
    max_value{max_value_}
{}

void Gui::show_stats(const Stats &stats)
{
	for (const auto &stat_index : stats.get_enabled_stats())
	{
		// Find the graph data of this stat index
		auto pr = stats_view.graph_map.find(stat_index);
		if (pr == stats_view.graph_map.end())
		{
			ImGui::Text("Stat not found");
			continue;
		}

		// Draw graph
		auto &      graph_data     = pr->second;
		const auto &graph_elements = stats.get_data(stat_index);
		float       graph_min      = 0.0f;
		float &     graph_max      = graph_data.max_value;

		if (!graph_data.has_fixed_max)
		{
			auto new_max = *std::max_element(graph_elements.begin(), graph_elements.end()) * stats_view.top_padding;
			if (new_max > graph_max)
			{
				graph_max = new_max;
			}
		}

		const ImVec2 graph_size = ImVec2{
		    ImGui::GetIO().DisplaySize.x,
		    stats_view.graph_height /* dpi */ * dpi_factor};

		char  graph_label[64];
		float avg = std::accumulate(graph_elements.begin(), graph_elements.end(), 0.0f) / graph_elements.size();

		// Check if the stat is available in the current platform
		if (!stats.is_available(stat_index))
		{
			std::sprintf(graph_label, "Stat not available");
		}
		else
		{
			std::sprintf(graph_label, graph_data.graph_label_format.c_str(), avg * graph_data.scale_factor);
		}

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PlotLines("", &graph_elements[0], static_cast<int>(graph_elements.size()), 0, graph_label, graph_min, graph_max, graph_size);
		ImGui::PopItemFlag();
	}
}

void Gui::show_options_window(std::function<void()> body, const uint32_t lines)
{
	// Add padding around the text so that the options are not
	// too close to the edges and are easier to interact with.
	// Also add double vertical padding to avoid rounded corners.
	const float window_padding = ImGui::CalcTextSize("T").x;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{window_padding, window_padding * 2.0f});
	auto window_height = lines * ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y * 2.0f;
	auto window_width  = ImGui::GetIO().DisplaySize.x;
	ImGui::SetNextWindowBgAlpha(overlay_alpha);
	const ImVec2 size = ImVec2(window_width, 0);
	ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	const ImVec2 pos = ImVec2(0.0f, ImGui::GetIO().DisplaySize.y - window_height);
	ImGui::SetNextWindowPos(pos, ImGuiSetCond_Always);
	const ImGuiWindowFlags flags   = (ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_AlwaysUseWindowPadding |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav);
	bool                   is_open = true;
	ImGui::Begin("Options", &is_open, flags);
	body();
	ImGui::End();
	ImGui::PopStyleVar();
}

bool Gui::input_event(const InputEvent &input_event)
{
	auto &io                 = ImGui::GetIO();
	auto  capture_move_event = false;

	if (input_event.get_source() == EventSource::Mouse)
	{
		const auto &mouse_button = static_cast<const MouseButtonInputEvent &>(input_event);

		io.MousePos = ImVec2{mouse_button.get_pos_x(),
		                     mouse_button.get_pos_y()};

		auto button_id = static_cast<int>(mouse_button.get_button());

		if (mouse_button.get_action() == MouseAction::Down)
		{
			io.MouseDown[button_id] = true;
		}
		else if (mouse_button.get_action() == MouseAction::Up)
		{
			io.MouseDown[button_id] = false;
		}
		else if (mouse_button.get_action() == MouseAction::Move)
		{
			capture_move_event = io.WantCaptureMouse;
		}
	}
	else if (input_event.get_source() == EventSource::Touchscreen)
	{
		const auto &touch_event = static_cast<const TouchInputEvent &>(input_event);

		io.MousePos = ImVec2{touch_event.get_pos_x(), touch_event.get_pos_y()};

		if (touch_event.get_action() == TouchAction::Down)
		{
			io.MouseDown[touch_event.get_pointer_id()] = true;
		}
		else if (touch_event.get_action() == TouchAction::Up)
		{
			io.MouseDown[touch_event.get_pointer_id()] = false;
		}
		else if (touch_event.get_action() == TouchAction::Move)
		{
			capture_move_event = io.WantCaptureMouse;
		}
	}

	if (!io.WantCaptureMouse)
	{
		// Hide/show GUI on tap/click outside GUI windows
		bool press_down = (input_event.get_source() == EventSource::Mouse && static_cast<const MouseButtonInputEvent &>(input_event).get_action() == MouseAction::Down) || (input_event.get_source() == EventSource::Touchscreen && static_cast<const TouchInputEvent &>(input_event).get_action() == TouchAction::Down);
		bool press_up   = (input_event.get_source() == EventSource::Mouse && static_cast<const MouseButtonInputEvent &>(input_event).get_action() == MouseAction::Up) || (input_event.get_source() == EventSource::Touchscreen && static_cast<const TouchInputEvent &>(input_event).get_action() == TouchAction::Up);

		if (press_down)
		{
			press_start = std::chrono::high_resolution_clock::now();
		}
		if (press_up)
		{
			auto press_end   = std::chrono::high_resolution_clock::now();
			auto press_delta = std::chrono::duration<double, std::milli>(press_end - press_start).count();
			if (press_delta < press_time_ms)
			{
				visible = !visible;
			}
		}
	}

	return capture_move_event;
}

}        // namespace vkb
