#!/bin/sh

modules=""
for ep in InitializeSignedDistanceField ConstructSignedDistanceField FinalizeSignedDistanceField CollideHairVerticesWithSdf_forward CollideHairVerticesWithSdf ; do
	glslangValidator -V TressFXCollisionSDF.comp.hlsl -e $ep -o TressFXCollisionSDF_$ep.comp.hlsl.spv
	modules="$modules TressFXCollisionSDF_$ep.comp.hlsl.spv"
done
# TODO combine $modules into TressFXCollisionSDF.comp.hlsl.spv

modules=""
for ep in IntegrationAndGlobalShapeConstraints VelocityShockPropagation LocalShapeConstraints LocalShapeConstraintsWithIteration LengthConstriantsWindAndCollision UpdateFollowHairVertices PrepareFollowHairBeforeTurningIntoGuide ; do
	glslangValidator -V TressFXSimulation.comp.hlsl -DAMD_TRESSFX_MAX_NUM_BONES=256 -e $ep -o TressFXSimulation_$ep.comp.hlsl.spv
	modules="$modules TressFXSimulation_$ep.comp.hlsl.spv"
done
# TODO combine $modules into TressFXSimulation.comp.hlsl.spv

modules=""
for ep in DepthsAlpha FillColors ResolveColor ResolveDepth ; do
	glslangValidator -V TressFXShortCut_$ep.frag.hlsl -e main -o TressFXShortCut_$ep.frag.hlsl.spv
	modules="$modules TressFXShortCut_$ep.frag.hlsl.spv"
done
# TODO combine $modules into TressFXShortCut.frag.hlsl

glslangValidator -V TressFXStrands_RenderHair.vert.hlsl -e VS_RenderHair_AA -o TressFXStrands_RenderHair.vert.hlsl.spv
