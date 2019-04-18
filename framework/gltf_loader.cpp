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

#define TINYGLTF_IMPLEMENTATION

#ifdef __ANDROID__
#	define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#include "gltf_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <queue>

#include "core/image.h"

#include "core/device.h"
#include "platform/thread_pool.h"

#include "scene_graph/components/perspective_camera.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"

namespace vkb
{
namespace
{
inline VkFilter find_min_filter(int minFilter)
{
	switch (minFilter)
	{
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
			return VK_FILTER_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			return VK_FILTER_LINEAR;
		default:
			return VK_FILTER_LINEAR;
	}
};

inline VkSamplerMipmapMode find_mipmap_mode(int minFilter)
{
	switch (minFilter)
	{
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
};

inline VkFilter find_mag_filter(int magFilter)
{
	switch (magFilter)
	{
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
			return VK_FILTER_NEAREST;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
			return VK_FILTER_LINEAR;
		default:
			return VK_FILTER_LINEAR;
	}
};

inline VkSamplerAddressMode find_wrap_mode(int wrap)
{
	switch (wrap)
	{
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		default:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
};

inline std::vector<uint8_t> get_attribute_data(const tinygltf::Model *model, uint32_t accessorId)
{
	auto &accessor   = model->accessors.at(accessorId);
	auto &bufferView = model->bufferViews.at(accessor.bufferView);
	auto &buffer     = model->buffers.at(bufferView.buffer);

	size_t stride    = accessor.ByteStride(bufferView);
	size_t startByte = accessor.byteOffset + bufferView.byteOffset;
	size_t endByte   = startByte + accessor.count * stride;

	return {buffer.data.begin() + startByte, buffer.data.begin() + endByte};
};

inline size_t get_attribute_size(const tinygltf::Model *model, uint32_t accessorId)
{
	return model->accessors.at(accessorId).count;
};

inline size_t get_attribute_stride(const tinygltf::Model *model, uint32_t accessorId)
{
	auto &accessor   = model->accessors.at(accessorId);
	auto &bufferView = model->bufferViews.at(accessor.bufferView);

	return accessor.ByteStride(bufferView);
};

inline VkFormat get_attribute_format(const tinygltf::Model *model, uint32_t accessorId)
{
	auto &accessor = model->accessors.at(accessorId);

	VkFormat format;

	switch (accessor.componentType)
	{
		case TINYGLTF_COMPONENT_TYPE_BYTE:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}};

			format = mappedFormat.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UINT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UINT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UINT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UINT}};

			static const std::map<int, VkFormat> mappedFormatNormalize = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UNORM},
			                                                              {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UNORM},
			                                                              {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UNORM},
			                                                              {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UNORM}};

			if (accessor.normalized)
			{
				format = mappedFormatNormalize.at(accessor.type);
			}
			else
			{
				format = mappedFormat.at(accessor.type);
			}

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_SHORT:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}};

			format = mappedFormat.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UINT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UINT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UINT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UINT}};

			static const std::map<int, VkFormat> mappedFormatNormalize = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UNORM},
			                                                              {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UNORM},
			                                                              {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UNORM},
			                                                              {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UNORM}};

			if (accessor.normalized)
			{
				format = mappedFormatNormalize.at(accessor.type);
			}
			else
			{
				format = mappedFormat.at(accessor.type);
			}

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_INT:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SINT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SINT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SINT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SINT}};

			format = mappedFormat.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_UINT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_UINT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_UINT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_UINT}};

			format = mappedFormat.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
		{
			static const std::map<int, VkFormat> mappedFormat = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SFLOAT},
			                                                     {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SFLOAT},
			                                                     {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SFLOAT},
			                                                     {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SFLOAT}};

			format = mappedFormat.at(accessor.type);

			break;
		}
		default:
			format = VK_FORMAT_UNDEFINED;
			break;
	}

	return format;
};

inline std::vector<uint8_t> convert_data(const std::vector<uint8_t> &srcData, uint32_t srcStride, uint32_t dstStride)
{
	auto elem_count = to_u32(srcData.size()) / srcStride;

	std::vector<uint8_t> result(elem_count * dstStride);

	for (uint32_t idxSrc = 0, idxDst = 0;
	     idxSrc < srcData.size() && idxDst < result.size();
	     idxSrc += srcStride, idxDst += dstStride)
	{
		std::copy(srcData.begin() + idxSrc, srcData.begin() + idxSrc + srcStride, result.begin() + idxDst);
	}

	return result;
}

inline void upload_image(CommandBuffer &command_buffer, core::Buffer &data, core::Image &image, ImageView &image_view)
{
	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memory_barrier.src_access_mask = 0;
		memory_barrier.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_HOST_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;

		command_buffer.image_memory_barrier(image_view, memory_barrier);
	}

	VkBufferImageCopy buffer_copy_region{};
	buffer_copy_region.imageSubresource.layerCount = image_view.get_subresource_range().layerCount;
	buffer_copy_region.imageSubresource.aspectMask = image_view.get_subresource_range().aspectMask;
	buffer_copy_region.imageExtent                 = image.get_extent();

	command_buffer.copy_buffer_to_image(data, image, {buffer_copy_region});

	{
		ImageMemoryBarrier memory_barrier{};
		memory_barrier.old_layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memory_barrier.new_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		memory_barrier.src_access_mask = 0;
		memory_barrier.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
		memory_barrier.src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		memory_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		command_buffer.image_memory_barrier(image_view, memory_barrier);
	}
}
}        // namespace

GLTFLoader::GLTFLoader(Device &device) :
    device{device}
{
}

bool GLTFLoader::read_scene_from_file(const std::string &file_name, sg::Scene &scene)
{
	std::string err;
	std::string warn;

	tinygltf::TinyGLTF gltf_loader;

	std::string gltf_file;

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
	gltf_file += "assets/";
#endif

	gltf_file += file_name;

	bool importResult = gltf_loader.LoadASCIIFromFile(&model, &err, &warn, gltf_file.c_str());

	if (!importResult)
	{
		LOGE("Failed to load gltf file {}.", gltf_file.c_str());

		return false;
	}

	if (!err.empty())
	{
		LOGE("Error loading gltf model: {}.", err.c_str());

		return false;
	}

	if (!warn.empty())
	{
		LOGI("{}", warn.c_str());
	}

	size_t pos = file_name.find_last_of('/');

	model_path = file_name.substr(0, pos);

	if (pos == std::string::npos)
	{
		model_path.clear();
	}

	scene = load_scene();

	return true;
}

sg::Scene GLTFLoader::load_scene()
{
	auto scene = sg::Scene();

	scene.set_name("gltf_scene");

	ThreadPool thread_pool;

	// Load samplers
	std::vector<std::unique_ptr<sg::Sampler>> sampler_components(model.samplers.size());

	for (size_t sampler_index = 0; sampler_index < model.samplers.size(); sampler_index++)
	{
		thread_pool.run(
		    [&](size_t sampler_index) {
			    auto sampler = parse_sampler(model.samplers.at(sampler_index));

			    sampler_components[sampler_index] = std::move(sampler);
		    },
		    sampler_index);
	}

	thread_pool.wait();

	scene.set_components(std::move(sampler_components));

	auto start_time = std::chrono::high_resolution_clock::now();

	// Load images
	std::vector<std::unique_ptr<sg::Image>> image_components(model.images.size());

	for (size_t image_index = 0; image_index < model.images.size(); image_index++)
	{
		thread_pool.run(
		    [&](size_t image_index) {
			    auto image = parse_image(model.images.at(image_index));

			    LOGI("Loaded gltf image #{} ({})", image_index, model.images.at(image_index).uri.c_str());

			    image_components[image_index] = std::move(image);
		    },
		    image_index);
	}

	thread_pool.wait();

	// Upload images to GPU
	std::vector<core::Buffer> transient_buffers;

	auto &command_buffer = device.request_command_buffer();

	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	for (size_t image_index = 0; image_index < image_components.size(); image_index++)
	{
		auto &image      = image_components.at(image_index);
		auto &gltf_image = model.images.at(image_index);

		core::Buffer stage_buffer{device, gltf_image.image.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
		stage_buffer.update(gltf_image.image);

		upload_image(command_buffer, stage_buffer, *image->image, *image->image_view);

		transient_buffers.push_back(std::move(stage_buffer));
	}

	command_buffer.end();

	auto &queue = device.get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);

	queue.submit(command_buffer, device.request_fence());

	device.get_fence_pool().wait();
	device.get_fence_pool().reset();
	device.get_command_pool().reset();

	transient_buffers.clear();

	scene.set_components(std::move(image_components));

	auto end_time = std::chrono::high_resolution_clock::now();

	auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

	LOGI("Time spent loading images: {} seconds.", elapsed_time.count());

	// Load textures
	auto images          = scene.get_components<sg::Image>();
	auto samplers        = scene.get_components<sg::Sampler>();
	auto default_sampler = create_default_sampler();

	for (auto &gltf_texture : model.textures)
	{
		auto texture = parse_texture(gltf_texture);

		texture->set_image(*images.at(gltf_texture.source));

		if (gltf_texture.sampler < 0)
		{
			texture->set_sampler(*default_sampler);
		}
		else
		{
			texture->set_sampler(*samplers.at(gltf_texture.sampler));
		}

		scene.add_component(std::move(texture));
	}

	scene.add_component(std::move(default_sampler));

	// Load materials
	auto textures = scene.get_components<sg::Texture>();

	for (auto &gltf_material : model.materials)
	{
		auto material = parse_material(gltf_material);

		for (auto &gltf_value : gltf_material.values)
		{
			if (gltf_value.first == "baseColorTexture")
			{
				material->base_color_texture = textures.at(gltf_value.second.TextureIndex());
			}
			else if (gltf_value.first == "metallicRoughnessTexture")
			{
				material->metallic_roughness_texture = textures.at(gltf_value.second.TextureIndex());
			}
		}

		for (auto &gltf_value : gltf_material.additionalValues)
		{
			if (gltf_value.first == "normalTexture")
			{
				material->normal_texture = textures.at(gltf_value.second.TextureIndex());
			}
			else if (gltf_value.first == "occlusionTexture")
			{
				material->occlusion_texture = textures.at(gltf_value.second.TextureIndex());
			}
			else if (gltf_value.first == "emissiveTexture")
			{
				material->emissive_texture = textures.at(gltf_value.second.TextureIndex());
			}
		}

		scene.add_component(std::move(material));
	}

	auto default_material = create_default_material();

	// Load meshes
	auto materials = scene.get_components<sg::PBRMaterial>();

	for (auto &gltf_mesh : model.meshes)
	{
		auto mesh = parse_mesh(gltf_mesh);

		for (auto &gltf_primitive : gltf_mesh.primitives)
		{
			auto submesh = parse_primitive(gltf_primitive);

			if (gltf_primitive.material < 0)
			{
				submesh->material = default_material.get();
			}
			else
			{
				submesh->material = materials.at(gltf_primitive.material);
			}

			mesh->add_submesh(*submesh);

			scene.add_component(std::move(submesh));
		}

		scene.add_component(std::move(mesh));
	}

	scene.add_component(std::move(default_material));

	// Load cameras
	for (auto &gltf_camera : model.cameras)
	{
		auto camera = parse_camera(gltf_camera);
		scene.add_component(std::move(camera));
	}

	// Load nodes
	auto meshes  = scene.get_components<sg::Mesh>();
	auto cameras = scene.get_components<sg::Camera>();

	std::vector<std::unique_ptr<sg::Node>> nodes;

	for (auto &gltf_node : model.nodes)
	{
		auto node = parse_node(gltf_node);

		if (gltf_node.mesh >= 0)
		{
			auto mesh = meshes.at(gltf_node.mesh);

			node->set_component(*mesh);

			mesh->add_node(*node);
		}

		if (gltf_node.camera >= 0)
		{
			auto camera = cameras.at(gltf_node.camera);

			node->set_component(*camera);

			camera->set_node(*node);
		}

		nodes.push_back(std::move(node));
	}

	// Load scenes
	std::queue<std::pair<sg::Node &, int>> traverse_nodes;

	for (auto &gltf_scene : model.scenes)
	{
		auto root_node = std::make_unique<sg::Node>(gltf_scene.name);

		for (auto node_index : gltf_scene.nodes)
		{
			traverse_nodes.push(std::make_pair(std::ref(*root_node), node_index));
		}

		while (!traverse_nodes.empty())
		{
			auto node_it = traverse_nodes.front();
			traverse_nodes.pop();

			auto &current_node = *nodes.at(node_it.second);
			auto &root_node    = node_it.first;

			current_node.set_parent(root_node);
			root_node.add_child(current_node);

			for (auto child_node_index : model.nodes[node_it.second].children)
			{
				traverse_nodes.push(std::make_pair(std::ref(root_node), child_node_index));
			}
		}

		scene.add_child(*root_node);
		nodes.push_back(std::move(root_node));
	}

	// Store nodes into the scene
	scene.set_nodes(std::move(nodes));

	// Create node for the default camera
	auto camera_node = std::make_unique<sg::Node>("default_camera");

	auto default_camera = create_default_camera();
	default_camera->set_node(*camera_node);
	camera_node->set_component(*default_camera);
	scene.add_component(std::move(default_camera));

	scene.add_child(*camera_node);
	scene.add_node(std::move(camera_node));

	return scene;
}

std::unique_ptr<sg::Node> GLTFLoader::parse_node(const tinygltf::Node &gltf_node)
{
	auto node = std::make_unique<sg::Node>(gltf_node.name);

	auto &transform = node->get_component<sg::Transform>();

	if (!gltf_node.translation.empty())
	{
		glm::vec3 translation;

		std::copy(gltf_node.translation.begin(), gltf_node.translation.end(), glm::value_ptr(translation));

		transform.set_translation(translation);
	}

	if (!gltf_node.rotation.empty())
	{
		glm::quat rotation;

		std::copy(gltf_node.rotation.begin(), gltf_node.rotation.end(), glm::value_ptr(rotation));

		transform.set_rotation(rotation);
	}

	if (!gltf_node.scale.empty())
	{
		glm::vec3 scale;

		std::copy(gltf_node.scale.begin(), gltf_node.scale.end(), glm::value_ptr(scale));

		transform.set_scale(scale);
	}

	if (!gltf_node.matrix.empty())
	{
		glm::mat4 matrix;

		std::copy(gltf_node.matrix.begin(), gltf_node.matrix.end(), glm::value_ptr(matrix));

		transform.set_matrix(matrix);
	}

	return node;
}

std::unique_ptr<sg::Camera> GLTFLoader::parse_camera(const tinygltf::Camera &gltf_camera)
{
	std::unique_ptr<sg::Camera> camera;

	if (gltf_camera.type == "perspective")
	{
		auto perspective_camera = std::make_unique<sg::PerspectiveCamera>(gltf_camera.name);

		perspective_camera->set_aspect_ratio(gltf_camera.perspective.aspectRatio);
		perspective_camera->set_field_of_view(gltf_camera.perspective.yfov);
		perspective_camera->set_near_plane(gltf_camera.perspective.znear);
		perspective_camera->set_far_plane(gltf_camera.perspective.zfar);

		camera = std::move(perspective_camera);
	}
	else
	{
		LOGW("Camera type not supported");
	}

	return camera;
}

std::unique_ptr<sg::Mesh> GLTFLoader::parse_mesh(const tinygltf::Mesh &gltf_mesh)
{
	return std::make_unique<sg::Mesh>(gltf_mesh.name);
}

std::unique_ptr<sg::SubMesh> GLTFLoader::parse_primitive(const tinygltf::Primitive &gltf_primitive)
{
	auto submesh = std::make_unique<sg::SubMesh>();

	for (auto &attribute : gltf_primitive.attributes)
	{
		std::string attrib_name = attribute.first;
		std::transform(attrib_name.begin(), attrib_name.end(), attrib_name.begin(), ::tolower);

		auto vertex_data = get_attribute_data(&model, attribute.second);

		if (attrib_name == "position")
		{
			submesh->vertices_count = to_u32(model.accessors.at(attribute.second).count);
		}

		core::Buffer buffer{device, vertex_data.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};

		buffer.update(vertex_data);

		auto pair = std::make_pair(attrib_name, std::move(buffer));

		submesh->vertex_buffers.insert(std::move(pair));

		sg::VertexAttribute attrib;
		attrib.format = get_attribute_format(&model, attribute.second);
		attrib.stride = to_u32(get_attribute_stride(&model, attribute.second));

		submesh->vertex_attributes[attrib_name] = attrib;
	}

	if (gltf_primitive.indices >= 0)
	{
		submesh->vertex_indices = to_u32(get_attribute_size(&model, gltf_primitive.indices));

		auto format = get_attribute_format(&model, gltf_primitive.indices);

		auto vertex_data = get_attribute_data(&model, gltf_primitive.indices);
		auto index_data  = get_attribute_data(&model, gltf_primitive.indices);

		switch (format)
		{
			case VK_FORMAT_R8_UINT:
				index_data = convert_data(index_data, 1, 2);

				submesh->index_type = VK_INDEX_TYPE_UINT16;
				break;
			case VK_FORMAT_R16_UINT:
				submesh->index_type = VK_INDEX_TYPE_UINT16;
				break;
			case VK_FORMAT_R32_UINT:
				submesh->index_type = VK_INDEX_TYPE_UINT32;
				break;
			default:
				LOGE("gltf primitive has invalid format type");
				break;
		}

		submesh->index_buffer = std::make_unique<core::Buffer>(device, index_data.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		submesh->index_buffer->update(index_data);
	}
	else
	{
		submesh->vertices_count = to_u32(get_attribute_size(&model, gltf_primitive.attributes.at("POSITION")));
	}

	return submesh;
}

std::unique_ptr<sg::PBRMaterial> GLTFLoader::parse_material(const tinygltf::Material &gltf_material)
{
	auto material = std::make_unique<sg::PBRMaterial>(gltf_material.name);

	for (auto &gltf_value : gltf_material.values)
	{
		if (gltf_value.first == "baseColorFactor")
		{
			const auto &color_factor    = gltf_value.second.ColorFactor();
			material->base_color_factor = glm::vec4(color_factor[0], color_factor[1], color_factor[2], color_factor[3]);
		}
		else if (gltf_value.first == "metallicFactor")
		{
			material->metallic_factor = static_cast<float>(gltf_value.second.Factor());
		}
		else if (gltf_value.first == "roughnessFactor")
		{
			material->roughness_factor = static_cast<float>(gltf_value.second.Factor());
		}
	}

	for (auto &gltf_value : gltf_material.additionalValues)
	{
		if (gltf_value.first == "emissiveFactor")
		{
			const auto &emissive_factor = gltf_value.second.number_array;
			material->emissive_factor   = glm::vec3(emissive_factor[0], emissive_factor[1], emissive_factor[2]);
		}
	}

	return material;
}

std::unique_ptr<sg::Image> GLTFLoader::parse_image(tinygltf::Image &gltf_image)
{
	auto image = std::make_unique<sg::Image>(gltf_image.name);

	int width  = gltf_image.width;
	int height = gltf_image.height;

	if (gltf_image.image.empty())
	{
		if (gltf_image.bufferView < 0)
		{
			const std::string image_file = model_path + "/" + gltf_image.uri;

			std::vector<uint8_t> compressed_data = read_binary_file(image_file);

			int comp, req_comp = 4;

			auto           data_size = to_u32(compressed_data.size());
			unsigned char *raw_data  = stbi_load_from_memory(reinterpret_cast<stbi_uc *>(compressed_data.data()),
                                                            data_size, &width, &height, &comp, req_comp);

			if (!raw_data)
			{
				LOGE("Failed to load image {}. Error: {}.", image_file.c_str(), stbi_failure_reason());

				return {};
			}

			gltf_image.image = {raw_data, raw_data + width * height * req_comp};

			free(raw_data);
		}
		else
		{
			throw std::runtime_error("Parsing embedded image not supported");
		}
	}

	image->image = std::make_unique<core::Image>(device, VkExtent3D{to_u32(width), to_u32(height), 1u}, VK_FORMAT_R8G8B8A8_UNORM,
	                                             VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	image->image_view = std::make_unique<ImageView>(*image->image, VK_IMAGE_VIEW_TYPE_2D);

	return image;
}

std::unique_ptr<sg::Sampler> GLTFLoader::parse_sampler(const tinygltf::Sampler &gltf_sampler)
{
	auto sampler = std::make_unique<sg::Sampler>(gltf_sampler.name);

	VkFilter minFilter = find_min_filter(gltf_sampler.minFilter);
	VkFilter magFilter = find_mag_filter(gltf_sampler.magFilter);

	VkSamplerAddressMode addressModeU = find_wrap_mode(gltf_sampler.wrapS);
	VkSamplerAddressMode addressModeV = find_wrap_mode(gltf_sampler.wrapT);
	VkSamplerAddressMode addressModeW = find_wrap_mode(gltf_sampler.wrapR);

	VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

	sampler_info.magFilter    = minFilter;
	sampler_info.minFilter    = minFilter;
	sampler_info.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.addressModeU = addressModeU;
	sampler_info.addressModeV = addressModeU;
	sampler_info.addressModeW = addressModeU;
	sampler_info.borderColor  = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VK_CHECK(vkCreateSampler(device.get_handle(), &sampler_info, nullptr, &sampler->vk_sampler));

	return sampler;
}

std::unique_ptr<sg::Texture> GLTFLoader::parse_texture(const tinygltf::Texture &gltf_texture)
{
	return std::make_unique<sg::Texture>(gltf_texture.name);
}

std::unique_ptr<sg::PBRMaterial> GLTFLoader::create_default_material()
{
	tinygltf::Material gltf_material;
	return parse_material(gltf_material);
}

std::unique_ptr<sg::Sampler> GLTFLoader::create_default_sampler()
{
	tinygltf::Sampler gltf_sampler;

	gltf_sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
	gltf_sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;

	gltf_sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
	gltf_sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
	gltf_sampler.wrapR = TINYGLTF_TEXTURE_WRAP_REPEAT;

	return parse_sampler(gltf_sampler);
}

std::unique_ptr<sg::Camera> GLTFLoader::create_default_camera()
{
	tinygltf::Camera gltf_camera;

	gltf_camera.name = "default_camera";
	gltf_camera.type = "perspective";

	gltf_camera.perspective.aspectRatio = 1.77f;
	gltf_camera.perspective.yfov        = 1.0f;
	gltf_camera.perspective.znear       = 0.1f;
	gltf_camera.perspective.zfar        = 1000.0f;

	return parse_camera(gltf_camera);
}
}        // namespace vkb
