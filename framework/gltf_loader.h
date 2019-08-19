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

#include <memory>
#include <mutex>

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include "timer.h"

namespace vkb
{
class Device;

namespace sg
{
class Scene;
class Node;
class Camera;
class Image;
class Mesh;
class PBRMaterial;
class Sampler;
class SubMesh;
class Texture;
}        // namespace sg

/**
 * @brief Helper Function to change array type T to array type Y
 * Create a struct that can be used with std::transform so that we do not need to recreate lambda functions
 * @param T 
 * @param Y 
 */
template <class T, class Y>
struct TypeCast
{
	Y operator()(T value) const noexcept
	{
		return static_cast<Y>(value);
	}
};

/// Read a gltf file and return a scene object. Converts the gltf objects
/// to our internal scene implementation. Mesh data is copied to vulkan buffers and
/// images are loaded from the folder of gltf file to vulkan images.
class GLTFLoader
{
  public:
	GLTFLoader(Device &device);

	virtual ~GLTFLoader() = default;

	std::unique_ptr<sg::Scene> read_scene_from_file(const std::string &file_name);

  protected:
	virtual std::unique_ptr<sg::Node> parse_node(const tinygltf::Node &gltf_node) const;

	virtual std::unique_ptr<sg::Camera> parse_camera(const tinygltf::Camera &gltf_camera) const;

	virtual std::unique_ptr<sg::Mesh> parse_mesh(const tinygltf::Mesh &gltf_mesh) const;

	virtual std::unique_ptr<sg::PBRMaterial> parse_material(const tinygltf::Material &gltf_material) const;

	virtual std::unique_ptr<sg::Image> parse_image(tinygltf::Image &gltf_image) const;

	virtual std::unique_ptr<sg::Sampler> parse_sampler(const tinygltf::Sampler &gltf_sampler) const;

	virtual std::unique_ptr<sg::Texture> parse_texture(const tinygltf::Texture &gltf_texture) const;

	virtual std::unique_ptr<sg::PBRMaterial> create_default_material();

	virtual std::unique_ptr<sg::Sampler> create_default_sampler();

	virtual std::unique_ptr<sg::Camera> create_default_camera();

	Device &device;

	tinygltf::Model model;

	std::string model_path;

  private:
	sg::Scene load_scene();
};
}        // namespace vkb
