/*
* Vulkan Example - AMD TressFX ported to Vulkan
*
* Copyright (C) 2018 by Gregor Riepl <onitake@gmail.com>
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "skeleton.h"

#include <stdexcept>
#include <limits>
#include <cstdio>
#include <cstring>
#include <fstream>

static const std::string SUSKELETON_MAGIC = "[SuSkeleton v2]";

size_t BoneMapping::addBone(const std::string& name, const glm::mat4 &matrix) {
	size_t index = indextoname.size();
	nametoindex[name] = index;
	indextoname.push_back(name);
	bonemtx.push_back(matrix);
	return index;
}

std::vector<float> BoneMapping::getBoneMatrices() const {
	std::vector<float> conv;
	for (auto it = bonemtx.begin(); it != bonemtx.end(); it++) {
		glm::mat4 m = *it;
		for (size_t i = 0; i < 4; i++) {
			conv.insert(conv.end(), &m[i][0], &m[i][3]);
		}
	}
	return conv;
}

void BoneMapping::loadSuSkeleton(const std::string &filename) {
	nametoindex.clear();
	indextoname.clear();
	bonemtx.clear();

	std::ifstream in;
	in.exceptions(std::ios::badbit);
	in.open(filename);
	unsigned long count = 0;
	std::string bonename;
	glm::mat4 matrix = glm::mat4(1.0);
	size_t pos = 0;
	enum {
		MAGIC,
		COUNT,
		BONE,
		MATRIX,
	} state = MAGIC;
	while (in.good()) {
		std::string line;
		std::getline(in, line);
		switch (state) {
			case MAGIC:
				if (line.compare(0, SUSKELETON_MAGIC.size(), SUSKELETON_MAGIC) == 0) {
					state = COUNT;
				} else {
					throw std::runtime_error("invalid magic");
					break;
				}
				break;
			case COUNT:
				count = std::stoul(line);
				state = BONE;
				break;
			case BONE:
				if (!line.empty()) {
					bonename = line;
					state = MATRIX;
				}
				break;
			case MATRIX:
				pos = 0;
				for (size_t col = 0; col < 4; col++) {
					for (size_t row = 0; row < 4; row++) {
						matrix[col][row] = std::stof(line, &pos);
					}
				}
				addBone(bonename, matrix);
				state = BONE;
				break;
		}
	}
}

// TressFX interface
// Note the use of unsigned int instead of size_t.
// We're guarding against possible type range mismatches.

unsigned int BoneMapping::GetBoneIndexByName(const char* pBoneName) const {
	if (nametoindex.find(pBoneName) == nametoindex.end()) {
		throw std::invalid_argument("invalid bone name");
	}
	size_t index = nametoindex.at(pBoneName);
	if (index > std::numeric_limits<unsigned int>::max()) {
		throw std::out_of_range("type-limited number of bones exceeded");
	}
	return static_cast<unsigned int>(index);
}

const char* BoneMapping::GetBoneNameByIndex(unsigned int index) const {
	return indextoname.at(index).c_str();
}

unsigned int BoneMapping::GetNumberOfBones() const {
	size_t size = nametoindex.size();
	if (size > std::numeric_limits<unsigned int>::max()) {
		throw std::out_of_range("type-limited number of bones exceeded");
	}
	return static_cast<unsigned int>(size);
}
