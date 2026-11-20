#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <vulkan/vulkan.hpp>

namespace shader {
	inline std::vector<char> readFile(std::string filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			std::cout << "failed to load \"" << filename << "\"" << std::endl;
		}
		size_t filesize{ static_cast<size_t>(file.tellg()) };
		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);
		file.close();
		return buffer;
	}
	inline vk::ShaderModule createModule(std::string filename, vk::Device device) {
		std::vector<char> code = readFile(filename);
		vk::ShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.flags = vk::ShaderModuleCreateFlags();
		moduleInfo.codeSize = code.size();
		moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		try {
			return device.createShaderModule(moduleInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "Failed to create shaders!";
		}
	}
}