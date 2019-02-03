
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
    Texture2D<float4> FragmentColorsTexture;
    Texture2D<float> tAccumInvAlpha;

    struct VS_OUTPUT_SCREENQUAD
    {
        float4 vPosition : SV_POSITION;
        float2 vTex      : TEXCOORD;
    };

    // Fourth pass of ShortCut.
    // Full-screen pass that finalizes the weighted average, and blends using the accumulated 1-alpha product.
    [earlydepthstencil]
    float4 main(VS_OUTPUT_SCREENQUAD input) : SV_Target
    {
        int2 vScreenAddress = int2(input.vPosition.xy);
        uint fragmentIndex = GetAddress(vScreenAddress);

        float fInvAlpha = tAccumInvAlpha[vScreenAddress];
        float fAlpha = 1.0 - fInvAlpha;

       if (fAlpha < SHORTCUT_MIN_ALPHA)
            return float4(0, 0, 0, 1);

        float4 fcolor;
        float colorSumX = FragmentColorsTexture[vScreenAddress].x;
        float colorSumY = FragmentColorsTexture[vScreenAddress].y;
        float colorSumZ = FragmentColorsTexture[vScreenAddress].z;
        float colorSumW = FragmentColorsTexture[vScreenAddress].w;
        fcolor.x = colorSumX / colorSumW;
        fcolor.y = colorSumY / colorSumW;
        fcolor.z = colorSumZ / colorSumW;
        fcolor.xyz *= fAlpha;
        fcolor.w = fInvAlpha;
        return fcolor;
    }

