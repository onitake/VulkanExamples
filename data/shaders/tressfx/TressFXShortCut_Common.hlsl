
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

#include "TressFXRendering.hlsl"

#define SHORTCUT_MIN_ALPHA 0.02

// Constants must match in TressFXShortCut.h

// Clear value for depths resource
#define SHORTCUT_INITIAL_DEPTH 0x3f800000

    float4x4 g_mInvViewProj;
    float4 g_vViewport;
    float3 g_vEye;

    float4 vFragmentBufferSize;

    // Convert 1D address to 2D address
    int2 GetAddress( int nAddress )
    {
        uint nBufferWidth = vFragmentBufferSize.x;
        int2 vAddress = int2( nAddress % nBufferWidth, nAddress / nBufferWidth );
        return vAddress;
    }

    // Convert 2D address to 1D address
    uint GetAddress( int2 vAddress )
    {
        int nAddress = vAddress.y * vFragmentBufferSize.x + vAddress.x;
        return nAddress;
    }

    uint PackFloat4IntoUint(float4 vValue)
    {
        return ( ((uint)(vValue.x*255)) << 24 ) | ( ((uint)(vValue.y*255)) << 16 ) | ( ((uint)(vValue.z*255)) << 8) | (uint)(vValue.w * 255);
    }

    float4 UnpackUintIntoFloat4(uint uValue)
    {
        return float4( ( (uValue & 0xFF000000)>>24 ) / 255.0, ( (uValue & 0x00FF0000)>>16 ) / 255.0, ( (uValue & 0x0000FF00)>>8 ) / 255.0, ( (uValue & 0x000000FF) ) / 255.0);
    }

    float3 ScreenPosToNDC(float3 vScreenPos, float4 viewport)
    {
        float2 xy = vScreenPos.xy;

        // add viewport offset.
        xy += viewport.xy;

        // scale by viewport to put in 0 to 1
        xy /= viewport.zw;

        // shift and scale to put in -1 to 1. y is also being flipped.
        xy.x = (2 * xy.x) - 1;
        xy.y = 1 - (2 * xy.y);

        return float3(xy, vScreenPos.z);
    }

float3 ScreenToNDC(float3 vScreenPos, float4 viewport)
{
	float2 xy = vScreenPos.xy;

	// add viewport offset.
	xy += viewport.xy;

	// scale by viewport to put in 0 to 1
	xy /= viewport.zw;

	// shift and scale to put in -1 to 1. y is also being flipped.
	xy.x = (2*xy.x) - 1;
	xy.y = 1 - (2*xy.y);

	return float3(xy, vScreenPos.z);

}

float3 NDCToWorld(float3 vNDC, float4x4 mInvViewProj)
{
	float4 pos = mul(mInvViewProj, float4(vNDC, 1) );
	
	return pos.xyz/pos.w;
}


// viewport.xy = offset, viewport.zw = size
float3 GetWorldPos(float4 vScreenPos, float4 viewport, float4x4 invViewProj)
{
	float2 xy = vScreenPos.xy;

	// add viewport offset.
	xy += viewport.xy;

	// scale by viewport to put in 0 to 1
	xy /= viewport.zw;

	// shift and scale to put in -1 to 1. y is also being flipped.
	xy.x = (2*xy.x) + 1;
	xy.y = 1 - (2*xy.y);

	float4 pos = mul(invViewProj, float4(xy.x, xy.y, vScreenPos.z, 1) );
	//float4 pos = float4(xy.x, xy.y, 1, 1);
	//pos *= sv_pos.w;
	
	return pos.xyz/pos.w;
}

