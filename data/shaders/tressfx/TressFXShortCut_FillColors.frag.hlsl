
//---------------------------------------------------------------------------------------
// Shader code related to ShortCut method.
//-------------------------------------------------------------------------------------
//
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "TressFXShortCut_Common.hlsl"
#include "TressFXPPLL_Common.hlsl"

#ifdef HEAD_SHADING_SUSHI

float4 ComputeSushiRGBA(float2 pixelCoord, float depth, float4 vTangentCoverage, float3 baseColor)
{
	float3 vTangent = 2.0 * vTangentCoverage.xyz - 1.0;
	float3 vNDC = ScreenToNDC(float3(pixelCoord, depth), g_vViewport);
	float3 vPositionWS = NDCToWorld(vNDC, g_mInvViewProj);
	float3 vViewWS = g_vEye - vPositionWS;

	// TODO remove params, since we are using globals anyways.
	HairShadeParams params;

	params.cColor = baseColor;
	params.fRadius = g_FiberRadius;
	params.fSpacing = g_FiberSpacing;
	params.fAlpha = g_HairShadowAlpha;

	float3 color = AccumulateHairLight(vTangent, vPositionWS, vViewWS, params);
	return float4(color, vTangentCoverage.w);
}

#define HEAD_SHADING ComputeSushiRGBA

#endif

#ifndef HEAD_SHADING

    float4 TressFXHeadColor(float2 pixelCoord, float depth, float4 vTangentCoverage, float3 baseColor)
    {
        return float4(baseColor, vTangentCoverage.w);
    }

#define HEAD_SHADING TressFXHeadColor

#endif

    struct PS_INPUT_HAIR_AA
    {
        float4 Position : SV_POSITION;
        float4 Tangent : Tangent;
        float4 p0p1 : TEXCOORD0;
        float3 strandColor : TEXCOORD1;
    };

    // Third pass of ShortCut.
    // Geometry pass that shades pixels passing early depth test.  Limited to near fragments due to previous depth write pass.
    // Colors are accumulated in render target for a weighted average in final pass.
    [earlydepthstencil]
    float4 main(PS_INPUT_HAIR_AA input) : SV_Target
    {
        float3 vNDC = ScreenPosToNDC(input.Position.xyz, g_vViewport);
        float coverage = ComputeCoverage(input.p0p1.xy, input.p0p1.zw, vNDC.xy, g_vViewport.zw);
        float alpha = coverage*g_MatBaseColor.a;

        ASSERT(coverage >= 0)
        if(alpha < SHORTCUT_MIN_ALPHA)
            return float4(0, 0, 0, 0);

        int2 vScreenAddress = int2(input.Position.xy);

        uint uDepth = asuint(input.Position.z);

        // Shade regardless of depth, since ResolveDepth pass writes one of the near depths

        float3 vPositionWS = NDCToWorld(vNDC, g_mInvViewProj);

        float4 rgbaColor = HEAD_SHADING(input.Position.xy, input.Position.z, float4(input.Tangent.xyz*0.5 + float3(0.5, 0.5, 0.5), alpha), input.strandColor.xyz);
        float3 color = rgbaColor.xyz;
        uint uColor = PackFloat4IntoUint(float4(color, alpha));

        return float4(color * alpha, alpha);
    }

