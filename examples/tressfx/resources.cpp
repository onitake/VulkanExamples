/*
* Vulkan Example - AMD TressFX ported to Vulkan
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "resources.h"

#include <string>
#include <vector>
#include <algorithm>

std::string getTressFXData(const std::string& name) {
	const std::string path = "data/models/tressfx/";
	return path + name;
}

std::string getTressFXTexture(const std::string &name) {
	const std::string path = "data/textures/tressfx/";
	return path + name;
}

void loadShaders(EI_LayoutManager *shaderman) {
	const std::string path = "data/shaders/tressfx/";
	shaderman->addShaderFile("IntegrationAndGlobalShapeConstraints", path + "TressFXSimulation_IntegrationAndGlobalShapeConstraints.comp.hlsl.spv");
	shaderman->addShaderFile("CollideHairVerticesWithSdf", path + "TressFXCollisionSDF_CollideHairVerticesWithSdf.comp.hlsl.spv");
	shaderman->addShaderFile("CollideHairVerticesWithSdf_forward", path + "TressFXCollisionSDF_CollideHairVerticesWithSdf_forward.comp.hlsl.spv");
	shaderman->addShaderFile("ConstructSignedDistanceField", path + "TressFXCollisionSDF_ConstructSignedDistanceField.comp.hlsl.spv");
	shaderman->addShaderFile("FinalizeSignedDistanceField", path + "TressFXCollisionSDF_FinalizeSignedDistanceField.comp.hlsl.spv");
	shaderman->addShaderFile("InitializeSignedDistanceField", path + "TressFXCollisionSDF_InitializeSignedDistanceField.comp.hlsl.spv");
	shaderman->addShaderFile("DepthsAlpha", path + "TressFXShortCut_DepthsAlpha.frag.hlsl.spv");
	shaderman->addShaderFile("FillColors", path + "TressFXShortCut_FillColors.frag.hlsl.spv");
	shaderman->addShaderFile("ResolveColor", path + "TressFXShortCut_ResolveColor.frag.hlsl.spv");
	shaderman->addShaderFile("ResolveDepth", path + "TressFXShortCut_ResolveDepth.frag.hlsl.spv");
	shaderman->addShaderFile("IntegrationAndGlobalShapeConstraints", path + "TressFXSimulation_IntegrationAndGlobalShapeConstraints.comp.hlsl.spv");
	shaderman->addShaderFile("LengthConstriantsWindAndCollision", path + "TressFXSimulation_LengthConstriantsWindAndCollision.comp.hlsl.spv");
	shaderman->addShaderFile("LocalShapeConstraints", path + "TressFXSimulation_LocalShapeConstraints.comp.hlsl.spv");
	shaderman->addShaderFile("LocalShapeConstraintsWithIteration", path + "TressFXSimulation_LocalShapeConstraintsWithIteration.comp.hlsl.spv");
	shaderman->addShaderFile("PrepareFollowHairBeforeTurningIntoGuide", path + "TressFXSimulation_PrepareFollowHairBeforeTurningIntoGuide.comp.hlsl.spv");
	shaderman->addShaderFile("UpdateFollowHairVertices", path + "TressFXSimulation_UpdateFollowHairVertices.comp.hlsl.spv");
	shaderman->addShaderFile("VelocityShockPropagation", path + "TressFXSimulation_VelocityShockPropagation.comp.hlsl.spv");
	shaderman->addShaderFile("VS_RenderHair_AA", path + "TressFXStrands_RenderHair.vert.hlsl.spv");
}

void applyLayouts(EI_Device *pDevice, EI_LayoutManager* shaderman) {
	CreateSimPosTanLayout2(pDevice, *shaderman);
	CreateRenderPosTanLayout2(pDevice, *shaderman);
	CreateRenderLayout2(pDevice, *shaderman);
	CreateGenerateSDFLayout2(pDevice, *shaderman);
	CreateSimLayout2(pDevice, *shaderman);
	CreateApplySDFLayout2(pDevice, *shaderman);
	CreateBoneSkinningLayout2(pDevice, *shaderman);
	CreateSDFMarchingCubesLayout2(pDevice, *shaderman);
	CreatePPLLBuildLayout2(pDevice, *shaderman);
	CreatePPLLReadLayout2(pDevice, *shaderman);
	CreateShortCutDepthsAlphaLayout2(pDevice, *shaderman);
	CreateShortCutResolveDepthLayout2(pDevice, *shaderman);
	CreateShortCutFillColorsLayout2(pDevice, *shaderman);
	CreateShortCutResolveColorLayout2(pDevice, *shaderman);
}

void loadBones(BoneMapping &mapper) {
	return; // this is now loaded from skeleton.txt
	std::vector<std::string> bones = {
		"frenchHornMonster_root_JNT",
		"frenchHornMonster_pelvis_JNT",
		"frenchHornMonster_spine01_JNT",
		"frenchHornMonster_spine02_JNT",
		"frenchHornMonster_spine03_JNT",
		"frenchHornMonster_neck_JNT",
		"frenchHornMonster_head_JNT",
		"frenchHornMonster_jaw_JNT",
		"frenchHornMonster_maineyebrow_JNT",
		"frenchHornMonster_L_maineyebrow_JNT",
		"frenchHornMonster_R_maineyebrow_JNT",
		"frenchHornMonster_L_eyebrow01_JNT",
		"frenchHornMonster_R_eyebrow01_JNT",
		"frenchHornMonster_L_LeyeLid_JNT",
		"frenchHornMonster_R_LeyeLid_JNT",
		"frenchHornMonster_L_eyeball_JNT",
		"frenchHornMonster_R_eyeball_JNT",
		"frenchHornMonster_L_clavicle_JNT",
		"frenchHornMonster_R_clavicle_JNT",
		"frenchHornMonster_L_ear_JNT",
		"frenchHornMonster_R_ear_JNT",
		"frenchHornMonster_L_UpperArm_JNT",
		"frenchHornMonster_R_UpperArm_JNT",
		"frenchHornMonster_L_ULeg_JNT",
		"frenchHornMonster_R_ULeg_JNT",
	};

	std::for_each(bones.begin(), bones.end(), [&mapper](const std::string &name) { mapper.addBone(name); });
}
