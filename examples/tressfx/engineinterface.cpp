/*
* Vulkan Example - AMD TressFX Vulkan engine interface
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <stdexcept>
#include <cstdio>
#include <regex>

#include "engineinterface.h"

#include "VulkanTools.h"

void VulkanResource::allocate(VulkanInterface *device)
{
	if (buffer || memory) {
		std::cout << "Warning : buffer/memory already allocated" << std::endl;
		return;
	}
	buffer = nullptr;
	memory = nullptr;
	VkBufferCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = size,
		.usage = 0,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
	};
	switch (type) {
		case TYPE_TEXEL:
			if (access == ACCESS_WRITEONLY) {
				info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
			} else {
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
			}
			break;
		case TYPE_UNIFORM:
			if (access == ACCESS_WRITEONLY) {
				info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			} else {
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			}
			break;
		case TYPE_INDEX:
			if (access == ACCESS_WRITEONLY) {
				info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			} else {
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			}
			break;
		case TYPE_VERTEX:
			if (access == ACCESS_WRITEONLY) {
				info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			} else {
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			}
			break;
		case TYPE_INDIRECT:
			if (access == ACCESS_WRITEONLY) {
				info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			} else {
				info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			}
			break;
		default:
			throw std::invalid_argument("invalid buffer type");
	}
	VK_CHECK_RESULT(vkCreateBuffer(device->getDevice(), &info, nullptr, &buffer));
	VkMemoryRequirements req = VkMemoryRequirements();
	vkGetBufferMemoryRequirements(device->getDevice(), buffer, &req);
	VkMemoryAllocateInfo alloc = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = req.size,
		.memoryTypeIndex = device->findMemoryType(req, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
	};
	VK_CHECK_RESULT(vkAllocateMemory(device->getDevice(), &alloc, nullptr, &memory));
}

void VulkanResource::destroy(VulkanInterface *device)
{
	vkFreeMemory(device->getDevice(), memory, nullptr);
	vkDestroyBuffer(device->getDevice(), buffer, nullptr);
}

void *VulkanResource::map(VulkanInterface *device)
{
	void *mappedmem;
	vkMapMemory(device->getDevice(), memory, 0, size, 0, &mappedmem);
	return mappedmem;
}

bool VulkanResource::unmap(VulkanInterface *device)
{
	vkUnmapMemory(device->getDevice(), memory);
	return true;
}

void VulkanResource::copyto(VulkanContext *context, VulkanResource *to)
{
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(context->getCommandBuffer(), buffer, to->buffer, 1, &copyRegion);
}

void VulkanShaderManager::addShaderFile(const std::string& name, const std::string& file) {
	FILE *fp = std::fopen(file.c_str(), "r");
	if (!fp) {
		throw std::invalid_argument("cannot open shader");
	}
	std::vector<uint32_t> shader;
	std::vector<uint32_t> buffer(256);
	while (!std::feof(fp)) {
		std::fread(buffer.data(), sizeof(uint32_t), buffer.size(), fp);
		shader.insert(shader.end(), buffer.begin(), buffer.end());
	}
	fclose(fp);
	shaders[name] = shader;
}

void VulkanContext::allocate(VkCommandPool pool) {
	VkDevice device = intf->getDevice();
	const VkCommandBufferAllocateInfo alloc = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &alloc, &cmd));
}

void VulkanContext::destroy(VkCommandPool pool) {
	VkDevice device = intf->getDevice();
	vkFreeCommandBuffers(device, pool, 1, &cmd);
	cmd = nullptr;
}

void VulkanShaderObject::allocate(VulkanInterface *device) {
	const VkShaderModuleCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = binary.size(),
		.pCode = binary.data(),
	};
	VK_CHECK_RESULT(vkCreateShaderModule(device->getDevice(), &info, nullptr, &module));
}

void VulkanShaderObject::destroy(VulkanInterface *device) {
	if (module != nullptr) {
		vkDestroyShaderModule(device->getDevice(), module, nullptr);
	}
}


// AMD TressFX interface follows

// Global render state (why global?????)
TressFXLayouts *g_TressFXLayouts;
// generated by VULKAN_EXAMPLE_MAIN(), defined in vulkanexamplebase.h
//VulkanExample *vulkanExample;

// Callbacks

void* tfxMalloc(size_t size)
{
	return std::calloc(1, size);
}

void tfxFree(void* p)
{
	std::free(p);
}

void tfxError(EI_StringHash message)
{
	std::cerr << "TressFX: " << message << std::endl;
}

void tfxRead(void* ptr, uint size, EI_Stream* pFile)
{
	FILE *f = reinterpret_cast<FILE *>(pFile);
	uint8_t *p = reinterpret_cast<uint8_t *>(ptr);
	size_t b = std::fread(p, 1, size, f);
	if (std::ferror(f))
		vks::tools::exitFatal("Error reading from stream", 1);
	if (b < size && std::feof(f))
		vks::tools::exitFatal("Didn't get enough bytes, EOF", 1);
}

void tfxSeek(EI_Stream* pFile, uint offset)
{
	FILE *f = reinterpret_cast<FILE *>(pFile);
	if (std::fseek(f, offset, SEEK_SET) == -1)
		vks::tools::exitFatal("Error seeking into stream", 1);
}

/////////////////////////////////////////////////////////////////////////////////
// Layouts
//
// Callbacks for the system that defines / assigns slots. 
// Bind sets, in the next section, the actual resources that are bound to these slots.

EI_BindLayout* tfxCreateLayout(EI_Device* pDevice,
	EI_LayoutManagerRef pLayoutManager,
	const AMD::TressFXLayoutDescription& description)
{
	// TODO implement shader stage layouts
	std::cout << "Warning : tfxCreateLayout is not implemented" << std::endl;
	EI_BindLayout *desc = new EI_BindLayout(description);
	//desc->allocate(pDevice);
	return desc;
	/*
	struct TressFXConstantBufferDesc
	{
		const EI_StringHash     constantBufferName;
		const int               bytes;
		const int               nConstants;
		const EI_StringHash*    parameterNames;
	};
	struct TressFXLayoutDescription
	{
		const int nSRVs; // = ShaderResourceView
		const EI_StringHash* srvNames;
		const int nUAVs; // = Unordered Access View
		const EI_StringHash* uavNames;
		TressFXConstantBufferDesc constants;
		EI_ShaderStage stage;
	};
	VkResult vkCreateBufferView(
		VkDevice
		const VkBufferViewCreateInfo*
		const VkAllocationCallbacks*
		VkBufferView*
		device,
		pCreateInfo,
		pAllocator,
		pView
	);
	*/
}

void tfxDestroyLayout(EI_Device* pDevice, EI_BindLayout* pLayout)
{
	// TODO implement shader stage layouts
	std::cout << "Warning : tfxDestroyLayout is not implemented" << std::endl;
	//pLayout->destroy(pDevice);
	delete pLayout;
}

/////////////////////////////////////////////////////////////////////////////////
// Bindsets
//
EI_BindSet* tfxCreateBindSet(EI_Device* pDevice, AMD::TressFXBindSet& bindSet)
{
	// TODO implement bind sets
	std::cout << "Warning : tfxCreateBindSet is not implemented" << std::endl;
	return nullptr;
	/*
	struct TressFXBindSet
	{
		int     nSRVs; // = ShaderResourceView
		EI_SRV* srvs;
		int     nUAVs; // = Unordered Access View
		EI_UAV* uavs;
		void*   values;
		int     nBytes;
	};
	*/
}

void tfxDestroyBindSet(EI_Device* pDevice, EI_BindSet* bindSet)
{
	std::cout << "Warning : tfxDestroyBindSet is not implemented" << std::endl;
}

void tfxBind(EI_CommandContextRef commandContext, EI_BindLayout* pLayout, EI_BindSet& bindSet)
{
	std::cout << "Warning : tfxBind is not implemented" << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
// Structured Buffers / Linear buffers
//
EI_Resource* tfxCreateROSB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName)
{
	EI_Resource *resource = new EI_Resource(resourceName, structSize * structCount, EI_Resource::TYPE_VERTEX, EI_Resource::ACCESS_WRITEONLY);
	resource->allocate(pDevice->vulkanInterface());
	return resource;
}

EI_Resource* tfxCreateRWSB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName)
{
	EI_Resource *resource = new EI_Resource(resourceName, structSize * structCount, EI_Resource::TYPE_VERTEX, EI_Resource::ACCESS_READWRITE);
	resource->allocate(pDevice->vulkanInterface());
	return resource;
}

void tfxDestroyResource(EI_Device* pDevice, EI_Resource* pResource)
{
	pResource->destroy(pDevice->vulkanInterface());
	delete pResource;
}

// Map gets a pointer to upload heap / mapped memory.
// Unmap issues the copy.
// This is always called on linear buffers.
void* tfxMap(EI_CommandContextRef pContext, EI_StructuredBufferRef sb)
{
	return sb.map(pContext.vulkanInterface());
}

bool tfxUnmap(EI_CommandContextRef pContext, EI_StructuredBufferRef sb)
{
	return sb.unmap(pContext.vulkanInterface());
}

EI_Resource* tfxCreateFP16RT(EI_Device* pDevice,
	const size_t     width,
	const size_t     height,
	const size_t     channels,
	EI_StringHash    strHash,
	float            clearR,
	float            clearG,
	float            clearB,
	float            clearA)
{
	// TODO create a half-float render target plus buffer
	std::cout << "Warning : tfxCreateFP16RT is not implemented" << std::endl;
	return nullptr;
}

EI_Resource* tfxCreateRW2D(EI_Device* pDevice,
	const size_t     width,
	const size_t     height,
	const size_t     arraySize,
	EI_StringHash    strHash)
{
	EI_Resource *resource = new EI_Resource(strHash, width * height * arraySize * sizeof(int32_t), EI_Resource::TYPE_TEXEL, EI_Resource::ACCESS_READWRITE);
	resource->allocate(pDevice->vulkanInterface());
	return resource;
}

EI_Resource* tfxCreateCountedSB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName)
{
	// no idea how to implement a "counter" for a buffer...
	// therefore let's just allocate a regular buffer for vertex usage.
	// if something like a unique id is needed, implement it in the shader using gl_GlobalInvocationID or similar.
	// "counters" are a complex beast in parallel computing and should probably only ever be used
	// for debugging and performance tuning purposes.
	std::cout << "Warning : global counters aren't supported in vulkan, tfxCreateCountedSB will create a standard vertex buffer instead" << std::endl;
	EI_Resource *resource = new EI_Resource(resourceName, structSize * structCount, EI_Resource::TYPE_VERTEX, EI_Resource::ACCESS_READWRITE);
	resource->allocate(pDevice->vulkanInterface());
	return resource;
}

// TODO get rid of clearValue argument?
void tfxClearCounter(
	EI_CommandContextRef    pContext,
	EI_StructuredBufferRef sb,
	AMD::uint32          clearValue)
{
	// now how are we supposed to set a "counter" on a buffer?
	std::cout << "Warning : global counters aren't supported in vulkan, tfxClearCounter will not work" << std::endl;
}

void tfxCopy(EI_CommandContextRef pContext, EI_StructuredBufferRef from, EI_StructuredBufferRef to)
{
	from.copyto(&pContext, &to);
}

void tfxClearRW2D(EI_CommandContext* pContext, EI_Resource* pResource, AMD::uint32 clearValue)
{
	// TODO clear the buffer
	std::cout << "Warning : tfxClearRW2D is not implemented" << std::endl;
}

void tfxSubmitBarriers(EI_CommandContextRef commands,
	int numBarriers,
	AMD::EI_Barrier* barriers)
{
	/*
	struct EI_Barrier
    {
        EI_Resource* pResource;
        EI_ResourceState from;
        EI_ResourceState to;
    };
	*/
	std::cout << "Warning : tfxSubmitBarriers is not implemented" << std::endl;
}

EI_PSO* tfxCreateComputeShaderPSO(EI_Device* pDevice,
	EI_LayoutManagerRef     layoutManager,
	const EI_StringHash& entryPoint)
{
	EI_PSO *shader = new EI_PSO(layoutManager.get(entryPoint));
	shader->allocate(pDevice->vulkanInterface());
	return shader;
}

void tfxDestroyPSO(EI_Device* pDevice, EI_PSO* pso)
{
	pso->destroy(pDevice->vulkanInterface());
	delete pso;
}

// All our compute shaders have dimensions of (N,1,1)
void tfxDispatch(EI_CommandContextRef commandContext, EI_PSO& pso, int nThreadGroups)
{
	// TODO execute the compute shader
	std::cout << "Warning : tfxDispatch is not implemented" << std::endl;
}

///////////////////////////////////////////////////////////////
// Indexed, instanced draw.
//
// Initialize and leave in state for use as index buffer.
// Indices are assumed to be 32 bits / 4 bytes.
// TODO upload should be seperate, and use command context.
EI_IndexBuffer* tfxCreateIndexBuffer(EI_Device* pDevice,
	AMD::uint32      size,
	void*            pInitialData, EI_StringHash objectName)
{
	EI_IndexBuffer *resource = new EI_IndexBuffer(objectName, size * sizeof(uint32_t), EI_IndexBuffer::TYPE_INDEX, EI_IndexBuffer::ACCESS_WRITEONLY);
	resource->allocate(pDevice->vulkanInterface());
	void *target = resource->map(pDevice->vulkanInterface());
	std::memcpy(target, pInitialData, size * sizeof(uint32_t));
	resource->unmap(pDevice->vulkanInterface());
	return resource;
}

void tfxDestroyIB(EI_Device* pDevice, EI_IndexBuffer* pBuffer)
{
	pBuffer->destroy(pDevice->vulkanInterface());
	delete pBuffer;
}

// TODO I don't think we are using the layouts and sets.  If we were, they would probaly be part of
// the pso we are already passing in.
// TODO This is probaly not the right place to have the PSO. It should actually be set before this.
// But Sushi is using it (because it needs technique name) to make the draw.
// TODO ...  so perhaps we need to define an "indexed draw" context.  Like in vulkan, it might
// contain the pipeline layout info.
void tfxDraw(EI_CommandContextRef     commandContext,
	EI_PSO&                pso,
	AMD::EI_IndexedDrawParams& drawParams)
{
	std::cout << "Warning : tfxDraw is not implemented" << std::endl;
}

// Functional interface
static struct SetupGlobalFunctionPointers {
	SetupGlobalFunctionPointers() {
		g_TressFXLayouts = new TressFXLayouts();
		std::memset(g_TressFXLayouts, 0, sizeof(*g_TressFXLayouts));
		AMD::g_Callbacks.pfError = tfxError;
		AMD::g_Callbacks.pfMalloc = tfxMalloc;
		AMD::g_Callbacks.pfFree = tfxFree;
		AMD::g_Callbacks.pfRead = tfxRead;
		AMD::g_Callbacks.pfSeek = tfxSeek;
		AMD::g_Callbacks.pfCreateLayout = tfxCreateLayout;
		AMD::g_Callbacks.pfDestroyLayout = tfxDestroyLayout;
		AMD::g_Callbacks.pfCreateBindSet = tfxCreateBindSet;
		AMD::g_Callbacks.pfDestroyBindSet = tfxDestroyBindSet;
		AMD::g_Callbacks.pfBind = tfxBind;
		AMD::g_Callbacks.pfSubmitBarriers = tfxSubmitBarriers;
		AMD::g_Callbacks.pfCreateReadOnlySB = tfxCreateROSB;
		AMD::g_Callbacks.pfCreateReadWriteSB = tfxCreateRWSB;
		AMD::g_Callbacks.pfDestroySB = tfxDestroyResource;
		AMD::g_Callbacks.pfMap = tfxMap;
		AMD::g_Callbacks.pfUnmap = tfxUnmap;
		AMD::g_Callbacks.pfCreateIndexBuffer = tfxCreateIndexBuffer;
		AMD::g_Callbacks.pfDestroyIB = tfxDestroyIB;
		AMD::g_Callbacks.pfDraw = tfxDraw;
		AMD::g_Callbacks.pfCreateComputeShaderPSO = tfxCreateComputeShaderPSO;
		AMD::g_Callbacks.pfDestroyPSO = tfxDestroyPSO;
		AMD::g_Callbacks.pfDispatch = tfxDispatch;
		AMD::g_Callbacks.pfCreate2D = tfxCreateRW2D;
		AMD::g_Callbacks.pfClear2D = tfxClearRW2D;
		AMD::g_Callbacks.pfCreateCountedSB = tfxCreateCountedSB;
		AMD::g_Callbacks.pfClearCounter = tfxClearCounter;
		AMD::g_Callbacks.pfCreateRT = tfxCreateFP16RT;
		AMD::g_Callbacks.pfCopy = tfxCopy;
	}
} setupFunctionPointers;
