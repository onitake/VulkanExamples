
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

    RWTexture2DArray<uint> RWFragmentDepthsTexture;

    struct PS_INPUT_HAIR_AA
    {
        float4 Position : SV_POSITION;
        float4 Tangent : Tangent;
        float4 p0p1 : TEXCOORD0;
        float3 strandColor : TEXCOORD1;
    };

    // First pass of ShortCut.
    // Geometry pass that stores the 3 front fragment depths, and accumulates product of 1-alpha values in the render target.
    [earlydepthstencil]
    float main(PS_INPUT_HAIR_AA input) : SV_Target
    {
        float3 vNDC = ScreenPosToNDC(input.Position.xyz, g_vViewport);
        float coverage = ComputeCoverage(input.p0p1.xy, input.p0p1.zw, vNDC.xy, g_vViewport.zw);
        float alpha = coverage*g_MatBaseColor.a;

        ASSERT(coverage >= 0)
        if (alpha < SHORTCUT_MIN_ALPHA)
            return 1.0;

        int2 vScreenAddress = int2(input.Position.xy);

        uint uDepth = asuint(input.Position.z);
        uint uDepth0Prev, uDepth1Prev;

        // Min of depth 0 and input depth
        // Original value is uDepth0Prev
        InterlockedMin(RWFragmentDepthsTexture[uint3(vScreenAddress, 0)], uDepth, uDepth0Prev);

        // Min of depth 1 and greater of the last compare
        // If fragment opaque, always use input depth (don't need greater depths)
        uDepth = (alpha > 0.98) ? uDepth : max(uDepth, uDepth0Prev);

        InterlockedMin(RWFragmentDepthsTexture[uint3(vScreenAddress, 1)], uDepth, uDepth1Prev);

        uint uDepth2Prev;

        // Min of depth 2 and greater of the last compare
        // If fragment opaque, always use input depth (don't need greater depths)
        uDepth = (alpha > 0.98) ? uDepth : max(uDepth, uDepth1Prev);

        InterlockedMin(RWFragmentDepthsTexture[uint3(vScreenAddress, 2)], uDepth, uDepth2Prev);

        return 1.0 - alpha;
    }

