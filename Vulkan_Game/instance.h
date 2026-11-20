#pragma once
#include <vulkan/vulkan.hpp>

namespace instance {
	bool supported(std::vector<const char*>& extensions, std::vector<const char*>& layers) {
		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
		bool found;
		for (const char* extension : extensions) {
			found = false;
			for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
				if (strcmp(extension, supportedExtension.extensionName) == 0) {
					found = true;
				}
			}
			if (!found) {
				return false;
			}
		}
		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

		for (const char* layer : layers) {
			found = false;
			for (vk::LayerProperties supportedLayer : supportedLayers) {
				if (strcmp(layer, supportedLayer.layerName) == 0) {
					found = true;
				}
			}
			if (!found) {
				return false;
			}
		}

		return true;
	}
	vk::Instance make_instance(const char* gameName) {
		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);
		version = VK_MAKE_API_VERSION(0, 1, 0, 0);
		vk::ApplicationInfo appInfo = vk::ApplicationInfo(
			gameName,
			version,
			"Vulkan Engine",
			version,
			version
		);
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		//extensions.push_back("VK_EXT_debug_utils");

		std::vector<const char*> layers;
		//layers.push_back("VK_LAYER_KHRONOS_validation");

		if (!supported(extensions, layers)) {
			return nullptr;
		}

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			static_cast<uint32_t>(layers.size()),
			layers.data(),
			static_cast<uint32_t>(extensions.size()),
			extensions.data()
		);
		try {
			return vk::createInstance(createInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "Failed to create Instance!\n";
			return nullptr;
		}
	}
}