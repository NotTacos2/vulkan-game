#pragma once
#include "image.h"
#include <vulkan/vulkan.hpp>

namespace image {
	class Texture {
	public:
		Texture(TextureInputChunk input);
		void use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayou);
		~Texture();
	private:
		int width, height, channels;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		const char* filename;
		stbi_uc* pixels;

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