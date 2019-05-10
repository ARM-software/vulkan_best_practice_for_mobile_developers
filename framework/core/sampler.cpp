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

#include "sampler.h"

#include "device.h"

namespace vkb
{
namespace core
{
Sampler::Sampler(Device &d, const VkSamplerCreateInfo &info) :
    device{d}
{
	VK_CHECK(vkCreateSampler(device.get_handle(), &info, nullptr, &handle));
}

Sampler::Sampler(Sampler &&other) :
    device{other.device},
    handle{other.handle}
{
	other.handle = VK_NULL_HANDLE;
}

Sampler::~Sampler()
{
	if (handle != VK_NULL_HANDLE)
	{
		vkDestroySampler(device.get_handle(), handle, nullptr);
	}
}

VkSampler Sampler::get_handle() const
{
	assert(handle != VK_NULL_HANDLE && "Sampler handle is invalid");
	return handle;
}
}        // namespace core
}        // namespace vkb
