/*
* Vulkan Example - AMD TressFX ported to Vulkan
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <iostream>
#include <map>
#include <string>
#include <cstdio>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gli/gli.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"

#include "vulkaninterface.h"
#include "engineinterface.h"
#include "skeleton.h"
#include "resources.h"
#include "TressFXAsset.h"
#include "TressFXHairObject.h"
#include "TressFXSimulation.h"
#include "TressFXSDFCollision.h"

#define ENABLE_VALIDATION false

class VulkanExample : public VulkanExampleBase, public VulkanInterface {
	VkPipeline pipeline;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	AMD::TressFXAsset *hairAsset;
	TressFXHairObject *tressfx;
	TressFXSimulation *simulation;
	TressFXSDFCollisionSystem *collisions;

	TressFXSimulationSettings parameters;
	BoneMapping skeleton;

	EI_Device *deviceWrapper;
	EI_Resource *hairColorTexture;
	EI_PSO *renderShader;

	EI_LayoutManager *shaderman;

	void loadHairData() {
		skeleton.loadSuSkeleton(getTressFXData("skeleton.txt"));;

		AMD::TressFXAsset* asset = new AMD::TressFXAsset();

		FILE *tfx = std::fopen(getTressFXData("Ratboy_mohawk.tfx").c_str(), "r");
		if (tfx == NULL) {
			vks::tools::exitFatal("can't open hair data asset", 1);
		}
		asset->LoadHairData(tfx);
		std::fclose(tfx);
		asset->GenerateFollowHairs(0, 0.0f, 0.0f);
		asset->ProcessAsset();

		FILE *tfxbone = std::fopen(getTressFXData("Ratboy_mohawk.tfxbone").c_str(), "r");
		if (tfxbone == NULL) {
			vks::tools::exitFatal("can't open bone data asset", 1);
		}
		asset->LoadBoneData(skeleton, tfxbone);
		std::fclose(tfxbone);

		hairAsset = asset;

		// this is actually the color texture of the skin surface where the hair strands
		// spawn from. the color sampled at the spawn point is used as the hair's base color.
		// note that this is used as a vertex shader input!
		// TODO convert data/textures/Ratboy_Body_MAT.tColor.tex to ktx to make this work
		std::string hairtexname = getTressFXTexture("Ratboy_Body_MAT.tColor.ktx");
		gli::texture hairtex = gli::load(hairtexname);
		if (hairtex.empty()) {
			vks::tools::exitFatal("can't load texture", 1);
		}
		gli::texture2d hairtex2d(hairtex);
		hairColorTexture = new EI_Resource("haircolor", hairtex2d.size(), VulkanResource::TYPE_TEXEL);
		hairColorTexture->allocate(this);
		void *mapping = hairColorTexture->map(this);
		std::memcpy(mapping, hairtex2d.data(), hairtex2d.size());
		hairColorTexture->unmap(this);
	}

	void allocateResources() {
		EI_CommandContext commandContext(this);
		commandContext.allocate(cmdPool);

		shaderman = new EI_LayoutManager();
		loadShaders(shaderman);
		applyLayouts(deviceWrapper, shaderman);

		TressFXHairObject* pObject = new TressFXHairObject();
		pObject->Create(hairAsset, deviceWrapper, commandContext, "hair", hairColorTexture);
		tressfx = pObject;

		renderShader = new EI_PSO(shaderman->get("RenderHair"));
		renderShader->allocate(this);

		commandContext.destroy(cmdPool);
	}
	
	void createSimulation() {
		simulation = new TressFXSimulation();
		simulation->Initialize(deviceWrapper, *shaderman);
		collisions = new TressFXSDFCollisionSystem();
		collisions->Initialize(deviceWrapper, *shaderman);
	}

	void transitionSimToRendering(EI_CommandContextRef context) {
		tressfx->GetPosTanCollection().TransitionSimToRendering(context);
	}
	void transitionRenderingToSim(EI_CommandContextRef context) {
		tressfx->GetPosTanCollection().TransitionRenderingToSim(context);
	}
	void updateBoneMatrices(EI_CommandContextRef context) {
		const std::vector<float> boneMatrices = skeleton.getBoneMatrices();
		// FIXME commandContext isn't even used
		tressfx->UpdateBoneMatrices(context, boneMatrices.data(), boneMatrices.size());
	}
	void simulate(EI_CommandContextRef context) {
		// FIXME missing function
		//tressfx->UpdateCapsuleCollisions();
		tressfx->UpdateSimulationParameters(parameters);
		simulation->Simulate(context, *tressfx);
	}

public:
	VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION) {
		zoom = -5.5f;
		zoomSpeed = 2.5f;
		rotationSpeed = 0.5f;
		rotation = { -0.5f, -112.75f, 0.0f };
		cameraPos = { 0.1f, 1.1f, 0.0f };
		title = "TressFX";
		settings.overlay = true;

		deviceWrapper = new EI_Device(this);
	}

	~VulkanExample() {
		// Clean up used Vulkan resources 
		// Note : Inherited destructor cleans up resources stored in base class

		if (tressfx) {
			tressfx->Destroy(deviceWrapper);
			delete tressfx;
		}
		if (hairAsset) {
			delete hairAsset;
		}
		if (hairColorTexture) {
			         hairColorTexture->destroy(this);
		}
		if (renderShader) {
			delete renderShader;
		}

		if (deviceWrapper) {
			delete deviceWrapper;
		}

		if (shaderman) {
			delete shaderman;
		}

		vkDestroyPipeline(device, pipeline, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}

	virtual void draw() {
		EI_CommandContext commandContext(this);
		commandContext.allocate(cmdPool);

		VulkanExampleBase::prepareFrame();

		updateBoneMatrices(commandContext);
		simulate(commandContext);

		tressfx->DrawStrands(commandContext, *renderShader);

		VulkanExampleBase::submitFrame();

		commandContext.destroy(cmdPool);
	}

	virtual void prepare() {
		VulkanExampleBase::prepare();

		loadHairData();
		allocateResources();

		prepared = true;
	}

	virtual void render() {
		if (!prepared) {
			return;
		}
		draw();
	}

	virtual void viewChanged() {
	}

	virtual void OnUpdateUIOverlay(vks::UIOverlay *overlay) {
		if (overlay->header("Settings")) {
			// TODO what?
		}
	}

	virtual VkDevice getDevice() {
		return device;
	}

	VkPhysicalDevice getPhysicalDevice() {
		return physicalDevice;
	}

	virtual uint32_t findMemoryType(const VkMemoryRequirements &req, VkMemoryPropertyFlags flags) {
		VkPhysicalDeviceMemoryProperties props = {};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &props);
		for (uint32_t i = 0; i < props.memoryTypeCount; i++) {
			if ((req.memoryTypeBits & (1 << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags) {
				return i;
			}
		}
		vks::tools::exitFatal("Couldn't find suitable memory", 1);
		return 0;
	}
};

VULKAN_EXAMPLE_MAIN()
