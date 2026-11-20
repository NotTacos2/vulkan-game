#include "vertex_menagerie.h"

VertexMenagerie::VertexMenagerie() {
	indexOffset = 0;
}

void VertexMenagerie::consume(memory::meshTypes type, std::vector<float>& vertexData, std::vector<uint32_t>& indexData) {
	int indexCount = static_cast<int>(indexData.size());
	int vertexCount = static_cast<int>(vertexData.size() / 11);
	int lastIndex = static_cast<int>(indexLump.size());

	firstIndices.insert(std::make_pair(type, lastIndex));
	indexCounts.insert(std::make_pair(type, indexCount));

	for (float attribute : vertexData) {
		vertexLump.push_back(attribute);
	}
	for (uint32_t index : indexData) {
		indexLump.push_back(index + indexOffset);
	}

	indexOffset += vertexCount;
}

void VertexMenagerie::finalize(vertexBufferFinalizationChunk finalizationChunk) {
	logicalDevice = finalizationChunk.logicalDevice;

	memory::BufferInputChunk inputChunk;
	inputChunk.logicalDevice = finalizationChunk.logicalDevice;
	inputChunk.physicalDevice = finalizationChunk.physicalDevice;
	inputChunk.size = sizeof(float) * vertexLump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	memory::Buffer stagingBuffer = memory::createBuffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertexLump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	vertexBuffer = memory::createBuffer(inputChunk);

	memory::copyBuffer(stagingBuffer, vertexBuffer, inputChunk.size, finalizationChunk.queue, finalizationChunk.commandBuffer);

	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);

	inputChunk.size = sizeof(uint32_t) * indexLump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	stagingBuffer = memory::createBuffer(inputChunk);

	memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, indexLump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	indexBuffer = memory::createBuffer(inputChunk);

	memory::copyBuffer(stagingBuffer, indexBuffer, inputChunk.size, finalizationChunk.queue, finalizationChunk.commandBuffer);

	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);
	vertexLump.clear();
}

VertexMenagerie::~VertexMenagerie() {
	logicalDevice.destroyBuffer(vertexBuffer.buffer);
	logicalDevice.freeMemory(vertexBuffer.bufferMemory);

	logicalDevice.destroyBuffer(indexBuffer.buffer);
	logicalDevice.freeMemory(indexBuffer.bufferMemory);
}