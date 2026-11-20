#include "frame.h"

void frame::SwapChainFrame::build_descriptor_resources() {
	memory::BufferInputChunk input;
	input.logicalDevice = logicalDevice;
	input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	input.physicalDevice = physicalDevice;
	input.size = sizeof(CameraVectors);
	input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	cameraVectorBuffer = memory::createBuffer(input);

	cameraVectorWriteLocation = logicalDevice.mapMemory(cameraVectorBuffer.bufferMemory, 0, sizeof(CameraVectors));

	input.size = sizeof(CameraMatrices);
	cameraMatrixBuffer = memory::createBuffer(input);

	cameraMatrixWriteLocation = logicalDevice.mapMemory(cameraMatrixBuffer.bufferMemory, 0, sizeof(CameraMatrices));

	input.size = 1024 * sizeof(glm::mat4);
	input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
	modelBuffer = memory::createBuffer(input);

	modelBufferWriteLocation = logicalDevice.mapMemory(modelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

	modelTransforms.reserve(1024);
	for (int i = 0; i < 1024; ++i) {
		modelTransforms.push_back(glm::mat4(1.0f));
	}

	cameraVectorDescriptor.buffer = cameraVectorBuffer.buffer;
	cameraVectorDescriptor.offset = 0;
	cameraVectorDescriptor.range = sizeof(CameraVectors);

	cameraMatrixDescriptor.buffer = cameraMatrixBuffer.buffer;
	cameraMatrixDescriptor.offset = 0;
	cameraMatrixDescriptor.range = sizeof(CameraMatrices);

	ssboDescriptor.buffer = modelBuffer.buffer;
	ssboDescriptor.offset = 0;
	ssboDescriptor.range = 1024 * sizeof(glm::mat4);
}

void frame::SwapChainFrame::build_depth_resources() {
	depthFormat = image::find_supported_format(physicalDevice, { vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	image::ImageInputChunk imageInfo;
	imageInfo.logicalDevice = logicalDevice;
	imageInfo.physicalDevice = physicalDevice;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.format = depthFormat;
	depthBuffer = image::make_image(imageInfo);
	depthBufferMemory = image::make_image_memory(imageInfo, depthBuffer);
	depthBufferView = image::make_image_view(logicalDevice, depthBuffer, depthFormat, vk::ImageAspectFlagBits::eDepth, vk::ImageViewType::e2D, 1);
}

void frame::SwapChainFrame::record_write_operations() {
	vk::WriteDescriptorSet cameraVectorWriteOp, cameraMatrixWriteOp, ssboWriteOp;

	cameraVectorWriteOp.dstSet = descriptorSet[pipelineType::SKY];
	cameraVectorWriteOp.dstBinding = 0;
	cameraVectorWriteOp.dstArrayElement = 0;
	cameraVectorWriteOp.descriptorCount = 1;
	cameraVectorWriteOp.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraVectorWriteOp.pBufferInfo = &cameraVectorDescriptor;

	cameraMatrixWriteOp.dstSet = descriptorSet[pipelineType::STANDARD];
	cameraMatrixWriteOp.dstBinding = 0;
	cameraMatrixWriteOp.dstArrayElement = 0;
	cameraMatrixWriteOp.descriptorCount = 1;
	cameraMatrixWriteOp.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraMatrixWriteOp.pBufferInfo = &cameraMatrixDescriptor;

	ssboWriteOp.dstSet = descriptorSet[pipelineType::STANDARD];
	ssboWriteOp.dstBinding = 1;
	ssboWriteOp.dstArrayElement = 0;
	ssboWriteOp.descriptorCount = 1;
	ssboWriteOp.descriptorType = vk::DescriptorType::eStorageBuffer;
	ssboWriteOp.pBufferInfo = &ssboDescriptor;

	writeOps = { cameraVectorWriteOp, cameraMatrixWriteOp, ssboWriteOp };
}

void frame::SwapChainFrame::write_descriptor_set() {
	logicalDevice.updateDescriptorSets(writeOps, nullptr);
}

void frame::SwapChainFrame::destroy() {
	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroyFramebuffer(framebuffer[pipelineType::SKY]);
	logicalDevice.destroyFramebuffer(framebuffer[pipelineType::STANDARD]);
	logicalDevice.destroyFence(inFlight);
	logicalDevice.destroySemaphore(imageAvailable);
	logicalDevice.destroySemaphore(renderFinished);

	logicalDevice.unmapMemory(cameraVectorBuffer.bufferMemory);
	logicalDevice.freeMemory(cameraVectorBuffer.bufferMemory);
	logicalDevice.destroyBuffer(cameraVectorBuffer.buffer);

	logicalDevice.unmapMemory(cameraMatrixBuffer.bufferMemory);
	logicalDevice.freeMemory(cameraMatrixBuffer.bufferMemory);
	logicalDevice.destroyBuffer(cameraMatrixBuffer.buffer);

	logicalDevice.unmapMemory(modelBuffer.bufferMemory);
	logicalDevice.freeMemory(modelBuffer.bufferMemory);
	logicalDevice.destroyBuffer(modelBuffer.buffer);

	logicalDevice.destroyImage(depthBuffer);
	logicalDevice.freeMemory(depthBufferMemory);
	logicalDevice.destroyImageView(depthBufferView);
}