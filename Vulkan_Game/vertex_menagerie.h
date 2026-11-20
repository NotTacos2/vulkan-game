#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include "memory.h"
#include <unordered_map>

struct vertexBufferFinalizationChunk {
	vk::Device logicalDevice;
	vk::PhysicalDevice physicalDevice;
	vk::CommandBuffer commandBuffer;
	vk::Queue queue;
};

class VertexMenagerie {
public:
	VertexMenagerie();
	~VertexMenagerie();
	void consume(memory::meshTypes type, std::vector<float>& vertexData, std::vector<uint32_t>& indexData);
	void finalize(vertexBufferFinalizationChunk finalizationChunk);
	memory::Buffer vertexBuffer, indexBuffer;
	std::unordered_map<memory::meshTypes, int> firstIndices;
	std::unordered_map<memory::meshTypes, int> indexCounts;
private:
	int indexOffset;
	vk::Device logicalDevice;
	std::vector<float> vertexLump;
	std::vector<uint32_t> indexLump;
};