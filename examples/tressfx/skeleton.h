/*
* Vulkan Example - AMD TressFX ported to Vulkan
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#ifndef _SKELETON_H
#define _SKELETON_H

#include <map>
#include <vector>
#include <string>

#include "TressFXBoneSkeletonInterface.h"

#define GLM_FORCE_RADIANS
#include <glm/mat4x4.hpp>

class BoneMapping : public TressFXSkeletonInterface {
	std::map<std::string, size_t> nametoindex;
	std::vector<std::string> indextoname;
	std::vector<glm::mat4> bonemtx;

public:
	BoneMapping() { }
	
	size_t addBone(const std::string &name, const glm::mat4 &matrix = glm::mat4(1.0));

	std::vector<float> getBoneMatrices() const;

	void loadSuSkeleton(const std::string &filename);

	// TressFX interface
	// Note the use of unsigned int instead of size_t.
	// We're guarding against possible type range mismatches.

	virtual unsigned int GetBoneIndexByName(const char* pBoneName) const;
	virtual const char* GetBoneNameByIndex(unsigned int index) const;
	virtual unsigned int GetNumberOfBones() const;
};

#endif //_SKELETON_H
