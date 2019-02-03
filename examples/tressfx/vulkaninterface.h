/*
* Vulkan Example - AMD TressFX Vulkan state interface
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#ifndef _VULKAN_INTERFACE_H
#define _VULKAN_INTERFACE_H

#include <vulkan/vulkan.h>

struct VulkanInterface {
	virtual VkDevice getDevice() = 0;
	virtual VkPhysicalDevice getPhysicalDevice() = 0;
	virtual uint32_t findMemoryType(const VkMemoryRequirements &req, VkMemoryPropertyFlags flags) = 0;
};

#endif /*_VULKAN_INTERFACE_H*/
