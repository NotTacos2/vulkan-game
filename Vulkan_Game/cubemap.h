#pragma once
#include <vulkan/vulkan.hpp>
#include "image.h"

namespace image {
	class CubeMap {
	public:
		CubeMap(TextureInputChunk input);
		void use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
		~CubeMap();
	private:
		int width, height, channels;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		std::vector<const char*> filenames;
		stbi_uc* pixels[6];

		vk::Image image;
		vk::DeviceMemory imageMemory;
		vk::ImageView imageView;
		vk::Sampler sampler;

		vk::DescriptorSetLayout layout;
		vk::DescriptorSet descriptorSet;
		vk::DescriptorPool descriptorPool;

		vk::CommandBuffer commandBuffer;
		vk::Queue queue;

		void load();
		void populate();
		void make_view();
		void make_sampler();
		void make_descriptor_set();
	};
}