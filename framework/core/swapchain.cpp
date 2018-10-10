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

#include "core/swapchain.h"

#include "device.h"

namespace vkb
{
namespace
{
inline uint32_t choose_image_count(
    uint32_t request_image_count,
    uint32_t min_image_count,
    uint32_t max_image_count)
{
	request_image_count = std::min(request_image_count, max_image_count);
	request_image_count = std::max(request_image_count, min_image_count);

	return request_image_count;
}

inline uint32_t choose_image_array_layers(
    uint32_t request_image_array_layers,
    uint32_t max_image_array_layers)
{
	request_image_array_layers = std::min(request_image_array_layers, max_image_array_layers);
	request_image_array_layers = std::max(request_image_array_layers, 1u);

	return request_image_array_layers;
}

inline VkExtent2D choose_extent(
    VkExtent2D        request_extent,
    const VkExtent2D &min_image_extent,
    const VkExtent2D &max_image_extent,
    const VkExtent2D &current_extent)
{
	if (request_extent.width < 1 || request_extent.height < 1)
	{
		LOGW("Requested image extent not supported. Selected default value.");

		return current_extent;
	}

	request_extent.width = std::max(request_extent.width, min_image_extent.width);
	request_extent.width = std::min(request_extent.width, max_image_extent.width);

	request_extent.height = std::max(request_extent.height, min_image_extent.height);
	request_extent.height = std::min(request_extent.height, max_image_extent.height);

	return request_extent;
}

inline VkPresentModeKHR choose_present_mode(
    VkPresentModeKHR                     request_present_mode,
    const std::vector<VkPresentModeKHR> &present_modes)
{
	auto present_mode_it = std::find(present_modes.begin(), present_modes.end(), request_present_mode);

	if (present_mode_it == present_modes.end())
	{
		present_mode_it = present_modes.begin();

		LOGW("Requested present mode not supported. Selected default value.");
	}

	return *present_mode_it;
}

inline VkSurfaceFormatKHR choose_surface_format(
    VkSurfaceFormatKHR                     request_surface_format,
    const std::vector<VkSurfaceFormatKHR> &surface_formats)
{
	auto surface_format_it = std::find_if(
	    surface_formats.begin(),
	    surface_formats.end(),
	    [&request_surface_format](const VkSurfaceFormatKHR &surface) {
		    if (surface.format == request_surface_format.format &&
		        surface.colorSpace == request_surface_format.colorSpace)
		    {
			    return true;
		    }

		    return false;
	    });

	if (surface_format_it == surface_formats.end())
	{
		surface_format_it = surface_formats.begin();

		LOGW("Requested surface format not supported. Selected default value.");
	}

	return *surface_format_it;
}

inline VkSurfaceTransformFlagBitsKHR choose_transform(
    VkSurfaceTransformFlagBitsKHR request_transform,
    VkSurfaceTransformFlagsKHR    supported_transform,
    VkSurfaceTransformFlagBitsKHR current_transform)
{
	if (request_transform & supported_transform)
	{
		return request_transform;
	}

	LOGW("Requested transform not supported. Selected default value.");

	return current_transform;
}

inline VkCompositeAlphaFlagBitsKHR choose_composite_alpha(VkCompositeAlphaFlagBitsKHR request_composite_alpha, VkCompositeAlphaFlagsKHR supported_composite_alpha)
{
	if (request_composite_alpha & supported_composite_alpha)
	{
		return request_composite_alpha;
	}

	LOGW("Requested composite alpha not supported. Selected default value.");

	static const std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
	    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
	    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
	    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};

	for (VkCompositeAlphaFlagBitsKHR composite_alpha : composite_alpha_flags)
	{
		if (composite_alpha & supported_composite_alpha)
		{
			return composite_alpha;
		}
	}

	throw std::runtime_error("No compatible composite alpha found.");
}

inline VkImageUsageFlags choose_image_usage(VkImageUsageFlags requested_image_usage, VkImageUsageFlags supported_image_usage)
{
	if (requested_image_usage & supported_image_usage)
	{
		return requested_image_usage;
	}

	static const std::vector<VkImageUsageFlagBits> image_usage_flags = {
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    VK_IMAGE_USAGE_STORAGE_BIT,
	    VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT};

	for (VkImageUsageFlagBits image_usage : image_usage_flags)
	{
		if (image_usage & supported_image_usage)
		{
			return image_usage;
		}
	}

	throw std::runtime_error("No compatible image usage found.");
}

inline VkImageUsageFlags composite_image_flags(const std::set<VkImageUsageFlagBits> &image_usage_flags)
{
	VkImageUsageFlags image_usage{};
	for (auto flag : image_usage_flags)
	{
		image_usage |= flag;
	}
	return image_usage;
}

}        // namespace

Swapchain::Swapchain(Swapchain &old_swapchain, const VkExtent2D &extent) :
    Swapchain{old_swapchain, old_swapchain.device, old_swapchain.surface, extent, old_swapchain.image_count, old_swapchain.transform, old_swapchain.present_mode, old_swapchain.image_usage}
{
}

Swapchain::Swapchain(Swapchain &old_swapchain, const uint32_t image_count) :
    Swapchain{old_swapchain, old_swapchain.device, old_swapchain.surface, old_swapchain.extent, image_count, old_swapchain.transform, old_swapchain.present_mode, old_swapchain.image_usage}
{
}

Swapchain::Swapchain(Swapchain &old_swapchain, const std::set<VkImageUsageFlagBits> &image_usage_flags) :
    Swapchain{old_swapchain, old_swapchain.device, old_swapchain.surface, old_swapchain.extent, old_swapchain.image_count, old_swapchain.transform, old_swapchain.present_mode, composite_image_flags(image_usage_flags)}
{
}

Swapchain::Swapchain(Swapchain &old_swapchain, const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform) :

    Swapchain{old_swapchain, old_swapchain.device, old_swapchain.surface, extent, old_swapchain.image_count, transform, old_swapchain.present_mode, old_swapchain.image_usage}

{
}

Swapchain::Swapchain(Device &                            device,
                     VkSurfaceKHR                        surface,
                     const VkExtent2D &                  extent,
                     const uint32_t                      image_count,
                     const VkSurfaceTransformFlagBitsKHR transform,
                     const VkPresentModeKHR              present_mode,
                     const VkImageUsageFlags             image_usage) :
    Swapchain{*this, device, surface, extent, image_count, transform, present_mode, image_usage}
{
}

Swapchain::Swapchain(Swapchain &                         old_swapchain,
                     Device &                            device,
                     VkSurfaceKHR                        surface,
                     const VkExtent2D &                  extent,
                     const uint32_t                      image_count,
                     const VkSurfaceTransformFlagBitsKHR transform,
                     const VkPresentModeKHR              present_mode,
                     const VkImageUsageFlags             image_usage) :
    device{device},
    surface{surface}
{
	VkSurfaceCapabilitiesKHR surface_capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->device.get_physical_device(), surface, &surface_capabilities);

	uint32_t surface_format_count{0U};
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(this->device.get_physical_device(), surface, &surface_format_count, nullptr));

	std::vector<VkSurfaceFormatKHR> surface_formats{surface_format_count};
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(this->device.get_physical_device(), surface, &surface_format_count, surface_formats.data()));

	uint32_t present_mode_count{0U};
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(this->device.get_physical_device(), surface, &present_mode_count, nullptr));

	std::vector<VkPresentModeKHR> present_modes{present_mode_count};
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(this->device.get_physical_device(), surface, &present_mode_count, present_modes.data()));

	VkSwapchainCreateInfoKHR create_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};

	create_info.minImageCount = choose_image_count(image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
	create_info.imageExtent   = choose_extent(extent, surface_capabilities.minImageExtent, surface_capabilities.maxImageExtent, surface_capabilities.currentExtent);

	VkSurfaceFormatKHR format = choose_surface_format({VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, surface_formats);

	create_info.imageFormat     = format.format;
	create_info.imageColorSpace = format.colorSpace;

	create_info.imageArrayLayers = choose_image_array_layers(1U, surface_capabilities.maxImageArrayLayers);
	create_info.imageUsage       = choose_image_usage(image_usage, surface_capabilities.supportedUsageFlags);

	create_info.preTransform   = choose_transform(transform, surface_capabilities.supportedTransforms, surface_capabilities.currentTransform);
	create_info.compositeAlpha = choose_composite_alpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surface_capabilities.supportedCompositeAlpha);
	create_info.presentMode    = choose_present_mode(present_mode, present_modes);

	create_info.surface      = surface;
	create_info.oldSwapchain = old_swapchain.get_handle();

	VkResult result = vkCreateSwapchainKHR(this->device.get_handle(), &create_info, nullptr, &handle);

	if (result != VK_SUCCESS)
	{
		throw VulkanException{result, "Cannot create Swapchain"};
	}

	uint32_t image_available{0u};
	VK_CHECK(vkGetSwapchainImagesKHR(this->device.get_handle(), handle, &image_available, nullptr));

	images.resize(image_available);

	VK_CHECK(vkGetSwapchainImagesKHR(this->device.get_handle(), handle, &image_available, images.data()));

	this->extent       = create_info.imageExtent;
	this->image_count  = create_info.minImageCount;
	this->transform    = create_info.preTransform;
	this->present_mode = create_info.presentMode;
	this->format       = create_info.imageFormat;
	this->image_usage  = create_info.imageUsage;
}

Swapchain::~Swapchain()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device.get_handle(), handle, nullptr);
	}
}

Swapchain::Swapchain(Swapchain &&other) :
    device{other.device},
    surface{other.surface},
    handle{other.handle},
    images{std::move(other.images)},
    extent{std::move(other.extent)},
    format{other.format},
    image_count{other.image_count},
    transform{other.transform},
    present_mode{other.present_mode}
{
	other.handle  = VK_NULL_HANDLE;
	other.surface = VK_NULL_HANDLE;
}

VkSwapchainKHR Swapchain::get_handle() const
{
	return handle;
}

VkResult Swapchain::acquire_next_image(uint32_t &image_index, VkSemaphore image_acquired_semaphore, VkFence fence)
{
	return vkAcquireNextImageKHR(device.get_handle(), handle, std::numeric_limits<uint64_t>::max(), image_acquired_semaphore, fence, &image_index);
}

const VkExtent2D &Swapchain::get_extent() const
{
	return extent;
}

const VkFormat &Swapchain::get_format() const
{
	return format;
}

const std::vector<VkImage> &Swapchain::get_images() const
{
	return images;
}
}        // namespace vkb
