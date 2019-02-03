
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

    Texture2DArray<uint> FragmentDepthsTexture;

    struct VS_OUTPUT_SCREENQUAD
    {
        float4 vPosition : SV_POSITION;
        float2 vTex      : TEXCOORD;
    };

    // Second pass of ShortCut.
    // Full-screen pass that writes the farthest of the near depths for depth culling.
    float main(VS_OUTPUT_SCREENQUAD input) : SV_Depth
    {
        // Blend the layers of fragments from back to front
        int2 vScreenAddress = int2(input.vPosition.xy);

        // Write farthest depth value for culling in the next pass.
        // It may be the initial value of 1.0 if there were not enough fragments to write all depths, but then culling not important.
        uint uDepth = FragmentDepthsTexture[uint3(vScreenAddress, 2)];

        return asfloat(uDepth);
    }

