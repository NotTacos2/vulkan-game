#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <set>
#include <iostream>
#include "shaders.h"
#include <glm/glm.hpp>
#include "memory.h"
#include "image.h"
#include "util.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace frame {
	struct UBO {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	struct CameraMatrices {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	struct CameraVectors {
		glm::vec4 forwards;
		glm::vec4 right;
		glm::vec4 up;
	};

	class SwapChainFrame {
	public:
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;

		vk::Image image;
		vk::ImageView imageView;
		std::unordered_map<pipelineType,vk::Framebuffer> framebuffer;
		vk::Image depthBuffer;
		vk::DeviceMemory depthBufferMemory;
		vk::ImageView depthBufferView;
		vk::Format depthFormat;
		int width, height;

		vk::CommandBuffer commandBuffer;

		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlight;

		CameraMatrices cameraMatrixData;
		memory::Buffer cameraMatrixBuffer;
		void* cameraMatrixWriteLocation;
		
		CameraVectors cameraVectorData;
		memory::Buffer cameraVectorBuffer;
		void* cameraVectorWriteLocation;

		std::vector<glm::mat4> modelTransforms;
		memory::Buffer modelBuffer;
		void* modelBufferWriteLocation;

		vk::DescriptorBufferInfo cameraVectorDescriptor, cameraMatrixDescriptor, ssboDescriptor;
		std::unordered_map<pipelineType, vk::DescriptorSet> descriptorSet;

		std::vector<vk::WriteDescriptorSet> writeOps;
		
		void build_descriptor_resources();

		void record_write_operations();

		void build_depth_resources();

		void write_descriptor_set();

		void destroy();
	};
}