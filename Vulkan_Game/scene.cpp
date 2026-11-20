#include "scene.h"

Scene::Scene() {
	positions.insert({ memory::meshTypes::GROUND, {} });
	positions.insert({ memory::meshTypes::GIRL, {} });
	positions.insert({ memory::meshTypes::CUBE, {} });
	positions[memory::meshTypes::GROUND].push_back(glm::vec3(10.0f, 0.0f, 0.0f));
	positions[memory::meshTypes::GIRL].push_back(glm::vec3(6.0f, 0.0f, 0.0f));
	positions[memory::meshTypes::CUBE].push_back(glm::vec3(8.0f, 1.5f, 0.0f));
}