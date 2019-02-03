/*
* Vulkan Example - AMD TressFX ported to Vulkan
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#ifndef _RESOURCES_H
#define _RESOURCES_H

#include "engineinterface.h"
#include "skeleton.h"

#include <string>
#include <cstdio>

std::string getTressFXData(const std::string &name);
std::string getTressFXTexture(const std::string &name);

void loadShaders(EI_LayoutManager *shaderman);
void applyLayouts(EI_Device *pDevice, EI_LayoutManager *shaderman);

void loadBones(BoneMapping &mapper);

#endif //_RESOURCES_H
