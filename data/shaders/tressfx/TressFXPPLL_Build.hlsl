
//---------------------------------------------------------------------------------------
// Shader code related to per-pixel linked lists.
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

#include "TressFXPPLL_Common.inc"

RWTexture2D<int>    tRWFragmentListHead;

RWStructuredBuffer<PPLL_STRUCT> LinkedListUAV;

int nNodePoolSize;

uint PackFloat4IntoUint(float4 vValue)
{
    return (((uint)(vValue.x * 255)) << 24) | (((uint)(vValue.y * 255)) << 16) | (((uint)(vValue.z * 255)) << 8) | (uint)(vValue.w * 255);
}

// Allocate a new fragment location in fragment color, depth, and link buffers
int AllocateFragment(int2 vScreenAddress)
{
    int newAddress = LinkedListUAV.IncrementCounter();
    if (newAddress <= 0 || newAddress > nNodePoolSize)
        newAddress = FRAGMENT_LIST_NULL;
    return newAddress;
}

// Insert a new fragment at the head of the list. The old list head becomes the
// the second fragment in the list and so on. Return the address of the *old* head.
int MakeFragmentLink(int2 vScreenAddress, int nNewHeadAddress)
{
    int nOldHeadAddress;

    InterlockedExchange(tRWFragmentListHead[vScreenAddress], nNewHeadAddress, nOldHeadAddress);

    return nOldHeadAddress;
}


// Write fragment attributes to list location. 
void WriteFragmentAttributes(int nAddress, int nPreviousLink, float4 vData, float3 vColor3, float fDepth)
{
    PPLL_STRUCT element;
    element.data = PackFloat4IntoUint(vData);
    element.color = PackFloat4IntoUint(float4(vColor3, 0));
    element.depth = asuint(saturate(fDepth));
    element.uNext = nPreviousLink;
    LinkedListUAV[nAddress] = element;
}

