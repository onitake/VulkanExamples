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
#include <stdexcept>
#include <limits>

#include "TressFXBoneSkeletonInterface.h"

#define GLM_FORCE_RADIANS
#include <glm/mat4x4.hpp>

class BoneMapping : public TressFXSkeletonInterface {
	std::map<std::string, size_t> nametoindex;
	std::vector<std::string> indextoname;
	std::vector<glm::mat4> bonemtx;

public:
	BoneMapping() { }
	
	size_t addbone(const std::string &name) {
		size_t index = indextoname.size();
		nametoindex[name] = index;
		indextoname.push_back(name);
		bonemtx.push_back(glm::mat4(1.0));
		return index;
	}

	const std::vector<float> getBoneMatrices() const {
		std::vector<float> conv;
		for (auto it = bonemtx.begin(); it != bonemtx.end(); it++) {
			glm::mat4 m = *it;
			for (size_t i = 0; i < 4; i++) {
				std::copy(&m[i][0], &m[i][3], conv.end());
			}
		}
		return conv;
	}

	// TressFX interface
	// Note the use of unsigned int instead of size_t.
	// We're guarding against possible type range mismatches.

	virtual unsigned int GetBoneIndexByName(const char* pBoneName) const {
		if (nametoindex.find(pBoneName) == nametoindex.end()) {
			throw std::invalid_argument("invalid bone name");
		}
		size_t index = nametoindex.at(pBoneName);
		if (index > std::numeric_limits<unsigned int>::max()) {
			throw std::out_of_range("type-limited number of bones exceeded");
		}
		return static_cast<unsigned int>(index);
	}
	virtual const char* GetBoneNameByIndex(unsigned int index) const {
		return indextoname.at(index).c_str();
	}
	virtual unsigned int GetNumberOfBones() const {
		size_t size = nametoindex.size();
		if (size > std::numeric_limits<unsigned int>::max()) {
			throw std::out_of_range("type-limited number of bones exceeded");
		}
		return static_cast<unsigned int>(size);
	}
};

#endif //_SKELETON_H
