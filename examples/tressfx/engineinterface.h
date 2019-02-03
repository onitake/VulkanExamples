/*
* Vulkan Example - AMD TressFX Vulkan engine interface
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#ifndef _ENGINE_INTERFACE_H
#define _ENGINE_INTERFACE_H

#include <string>
#include <vector>
#include <map>

#include "vulkaninterface.h"

#include "TressFXLayouts.h"

class VulkanContext {
	VkCommandBuffer cmd;
	VulkanInterface *intf;

public:
	VulkanContext(VulkanInterface *i) : intf(i) { }

	void allocate(VkCommandPool pool);
	void destroy(VkCommandPool pool);

	VulkanInterface *vulkanInterface() {
		return intf;
	}
	VkCommandBuffer getCommandBuffer() {
		return cmd;
	}
};

struct VulkanResource {
	enum Type {
		TYPE_TEXEL,
		TYPE_UNIFORM,
		TYPE_INDEX,
		TYPE_VERTEX,
		TYPE_INDIRECT,
	};
	// Access level from host side
	enum Access {
		ACCESS_WRITEONLY,
		ACCESS_READWRITE,
	};

	std::string name;
	size_t size;
	Type type;
	Access access;

	VkBuffer buffer;
	VkDeviceMemory memory;

	VulkanResource(const std::string &n, size_t s, Type t, Access a = ACCESS_WRITEONLY) :
		name(n), size(s), type(t), access(a), buffer(nullptr), memory(nullptr) { }
	~VulkanResource() { }

	void allocate(VulkanInterface *device);
	void destroy(VulkanInterface *device);
	void *map(VulkanInterface *device);
	bool unmap(VulkanInterface *device);
	void copyto(VulkanContext *context, VulkanResource *to);
};

class VulkanDevice {
	VulkanInterface *intf;
public:
	VulkanDevice(VulkanInterface *i) : intf(i) { }
	VulkanInterface *vulkanInterface() {
		return intf;
	}
};

struct VulkanBindingDescription {
	VulkanBindingDescription(const AMD::TressFXLayoutDescription&) {}
};

struct VulkanVertexInput {
};

class VulkanShaderObject {
	std::vector<uint32_t> binary;
	VkShaderModule module;

public:
	VulkanShaderObject(const std::vector<uint32_t> &code) : binary(code), module(nullptr) { }
	
	void allocate(VulkanInterface *device);
	void destroy(VulkanInterface *device);
};

class VulkanShaderManager {
	std::map<std::string, std::vector<uint32_t>> shaders;

public:
	VulkanShaderManager() { }

	void addShaderFile(const std::string& name, const std::string &file);

	std::vector<uint32_t> get(const std::string &entrypoint) {
		return shaders[entrypoint];
	}
};

class EI_Device : public VulkanDevice {
	using VulkanDevice::VulkanDevice;
};
class EI_Resource : public VulkanResource {
	using VulkanResource::VulkanResource;
};
class EI_CommandContext : public VulkanContext {
	using VulkanContext::VulkanContext;
};
class EI_LayoutManager : public VulkanShaderManager {
	using VulkanShaderManager::VulkanShaderManager;
};
struct EI_BindLayout : public VulkanBindingDescription {
	using VulkanBindingDescription::VulkanBindingDescription;
};
class EI_BindSet : public VulkanVertexInput { };
class EI_PSO : public VulkanShaderObject {
	using VulkanShaderObject::VulkanShaderObject;
};
class EI_IndexBuffer : public VulkanResource {
	using VulkanResource::VulkanResource;
};

#endif /*_ENGINE_INTERFACE_H*/
