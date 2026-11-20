#pragma once
#include <glm/glm.hpp>
#include "memory.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class Scene {
public:
	Scene();

	std::unordered_map <memory::meshTypes, std::vector<glm::vec3>> positions;
};
