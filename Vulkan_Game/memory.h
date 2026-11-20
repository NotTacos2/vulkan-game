#pragma once
#include <vulkan/vulkan.hpp>

namespace memory {
	struct BufferInputChunk {
		size_t size;
		vk::BufferUsageFlags usage;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		vk::MemoryPropertyFlags memoryProperties;
	};
	struct Buffer {
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;
	};
	enum class meshTypes {
		GIRL,
		GROUND,
		CUBE
	};
	uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
	void allocateBufferMemory(Buffer& buffer, const BufferInputChunk& input);
	Buffer createBuffer(BufferInputChunk input);
	void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, vk::DeviceSize size, vk::Queue queue, vk::CommandBuffer commandBuffer);
}