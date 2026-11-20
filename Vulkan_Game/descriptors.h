#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <iostream>

namespace descriptors {
	struct descriptorSetLayoutData {
		int count;
		std::vector<int> indices;
		std::vector<vk::DescriptorType> types;
		std::vector<int> counts;
		std::vector<vk::ShaderStageFlags> stages;
	};

	vk::DescriptorSetLayout build_descriptorsetlayout(
		vk::Device device, const descriptorSetLayoutData& bindings);

	vk::DescriptorPool build_descriptorpool(
		vk::Device device, uint32_t size, const descriptorSetLayoutData& bindings);

	vk::DescriptorSet allocate_descriptor_set(
		vk::Device device, vk::DescriptorPool descriptorPool,
		vk::DescriptorSetLayout layout);
}