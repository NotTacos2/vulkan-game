#pragma once
#include "memory.h"

class TriangleMesh {
public:
	TriangleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);
	~TriangleMesh();
	memory::Buffer vertexBuffer;
private:
	vk::Device logicalDevice;
};