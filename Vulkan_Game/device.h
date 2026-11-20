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
#include "frame.h"
#include "util.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace device {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct framebufferInput {
		vk::Device device;
		std::unordered_map<pipelineType, vk::RenderPass> renderpass;
		vk::Extent2D swapchainExtent;
	};

	struct SwapChainBundle {
		vk::SwapchainKHR swapchain;
		std::vector<frame::SwapChainFrame> frames;
		vk::Format format;
		vk::Extent2D extent;
	};

	struct commandBufferInputChunk {
		vk::Device device;
		vk::CommandPool commandPool;
		std::vector<frame::SwapChainFrame>& frames;
	};

	struct ObjectData {
		glm::mat4 model;
	};

	inline bool checkDeviceExtensionSupport(
		const vk::PhysicalDevice& device,
		const std::vector<const char*>& requestedExtensions
	) {
		std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());
		for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}
	inline bool isSuitable(const vk::PhysicalDevice& device) {
		const std::vector<const char*> requestedExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		bool extensionsSupported = checkDeviceExtensionSupport(device, requestedExtensions);
		if (extensionsSupported) {

		}
		else {
			return false;
		}
		return true;
	}
	inline vk::PhysicalDevice choosedevice(vk::Instance& instance) {
		std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();
		for (vk::PhysicalDevice device : availableDevices) {
			if (isSuitable(device)) {
				return device;
			}
			else {
				std::cout << "Your computer can't support this!";
			}
		}
		return nullptr;
	}
	inline QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
		QueueFamilyIndices indices;
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
		int i = 0;
		for (vk::QueueFamilyProperties queueFamliy : queueFamilies) {
			if (queueFamliy.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.graphicsFamily = i;
			}
			if (device.getSurfaceSupportKHR(i, surface)) {
				indices.presentFamily = i;
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}

		return indices;
	}
	inline vk::Device create_logical_device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		std::vector<uint32_t> uniqueIndices;
		uniqueIndices.push_back(indices.graphicsFamily.value());
		if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
			uniqueIndices.push_back(indices.presentFamily.value());
		}
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
		float queuePriority = 1.0f;
		for (uint32_t queueFamilyIndex : uniqueIndices) {
			queueCreateInfo.push_back(
				vk::DeviceQueueCreateInfo(
					vk::DeviceQueueCreateFlags(),
					queueFamilyIndex,
					1,
					&queuePriority
				)
			);
		}
		
		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();

		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::vector<const char*> enabledLayers;

		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			static_cast<uint32_t>(queueCreateInfo.size()),
			queueCreateInfo.data(),
			static_cast<uint32_t>(enabledLayers.size()),
			enabledLayers.data(),
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data(),
			&deviceFeatures
		);
		try {
			vk::Device device = physicalDevice.createDevice(deviceInfo);
			return device;
		}
		catch (vk::SystemError err) {
			std::cout << "Device creation failed\n";
			return nullptr;
		}
		return nullptr;
	}
	inline std::array<vk::Queue, 2> get_queue(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface) {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

		return { {
			device.getQueue(indices.graphicsFamily.value(), 0),
			device.getQueue(indices.presentFamily.value(), 0),
		} };
	}
	inline SwapChainSupportDetails query_swapchain_support(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
		SwapChainSupportDetails support;
		support.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		support.formats = device.getSurfaceFormatsKHR(surface);
		support.presentModes = device.getSurfacePresentModesKHR(surface);
		return support;
	}
	inline vk::SurfaceFormatKHR choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats) {
		for (vk::SurfaceFormatKHR format : formats) {
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return format;
			}
		}
		return formats[0];
	}
	inline vk::PresentModeKHR choose_swapchain_present_mode(std::vector<vk::PresentModeKHR> presentModes) {
		for (vk::PresentModeKHR presentMode : presentModes) {
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				return presentMode;
			}
		}
		return vk::PresentModeKHR::eFifo;
	}
	inline vk::Extent2D choose_swapchain_extent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			vk::Extent2D extent = { width, height };
			extent.width = std::min(
				capabilities.maxImageExtent.width,
				std::max(capabilities.minImageExtent.width, extent.width)
			);
			extent.height = std::min(
				capabilities.maxImageExtent.height,
				std::max(capabilities.minImageExtent.height, extent.height)
			);
			return extent;
		}
	}
	inline SwapChainBundle create_swapchain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, int width, int height) {
		SwapChainSupportDetails support = query_swapchain_support(physicalDevice, surface);
		vk::SurfaceFormatKHR format = choose_swapchain_surface_format(support.formats);
		vk::PresentModeKHR presentMode = choose_swapchain_present_mode(support.presentModes);
		vk::Extent2D extent = choose_swapchain_extent(width, height, support.capabilities);
		uint32_t imageCount = std::min(
			support.capabilities.maxImageCount,
			support.capabilities.minImageCount + 1
		);
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(),
			surface,
			imageCount,
			format.format,
			format.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment
		);
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);
		SwapChainBundle bundle{};
		try {
			bundle.swapchain = logicalDevice.createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("failed to create swap chain!");
		}
		std::vector<vk::Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
		bundle.frames.resize(images.size());

		for (size_t i = 0; i < images.size(); ++i) {
			bundle.frames[i].image = images[i];
			bundle.frames[i].imageView = image::make_image_view(logicalDevice, images[i], format.format, vk::ImageAspectFlagBits::eColor, vk::ImageViewType::e2D, 1);
		}

		bundle.format = format.format;
		bundle.extent = extent;

		return bundle;
	}

	inline vk::VertexInputBindingDescription getPosColorBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = 11 * sizeof(float);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		return bindingDescription;
	}

	inline std::vector<vk::VertexInputAttributeDescription> getPosColorAttributeDesciptions() {
		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::VertexInputAttributeDescription dummy;
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes.push_back(dummy);
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = vk::Format::eR32G32B32Sfloat;
		attributes[0].offset = 0;
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = vk::Format::eR32G32B32Sfloat;
		attributes[1].offset = 3 * sizeof(float);
		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].format = vk::Format::eR32G32Sfloat;
		attributes[2].offset = 6 * sizeof(float);
		attributes[3].binding = 0;
		attributes[3].location = 3;
		attributes[3].format = vk::Format::eR32G32B32Sfloat;
		attributes[3].offset = 8 * sizeof(float);
		return attributes;
	}

	inline void make_framebuffers(framebufferInput inputChunk, std::vector<frame::SwapChainFrame>& frames) {
		for (int i = 0; i < frames.size(); ++i) {
			std::vector<vk::ImageView> attachments = {
				frames[i].imageView
			};
			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = inputChunk.renderpass[pipelineType::SKY];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = inputChunk.swapchainExtent.width;
			framebufferInfo.height = inputChunk.swapchainExtent.height;
			framebufferInfo.layers = 1;

			try {
				frames[i].framebuffer[pipelineType::SKY] = inputChunk.device.createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError err) {
				std::cout << "Failed to create sky framebuffer for frame " << i << std::endl;
			}

			attachments.push_back(frames[i].depthBufferView);

			framebufferInfo.renderPass = inputChunk.renderpass[pipelineType::STANDARD];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();

			try {
				frames[i].framebuffer[pipelineType::STANDARD] = inputChunk.device.createFramebuffer(framebufferInfo);
			}
			catch (vk::SystemError err) {
				std::cout << "Failed to create standard framebuffer for frame " << i << std::endl;
			}
		}
	}

	inline vk::Semaphore make_semaphore(vk::Device device) {
		vk::SemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.flags = vk::SemaphoreCreateFlags();

		try {
			return device.createSemaphore(semaphoreInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "failed to create semaphore\n";
			return nullptr;
		}
	}

	inline vk::Fence make_fence(vk::Device device) {
		vk::FenceCreateInfo fenceInfo = {};
		fenceInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		try {
			return device.createFence(fenceInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "Failed to create fence\n";
			return nullptr;
		}
	}

	inline vk::CommandPool make_commandpool(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		try {
			return device.createCommandPool(poolInfo);
		}
		catch (vk::SystemError err) {
			std::cout << "failed to create command pool\n";
			return nullptr;
		}
	}

	inline vk::CommandBuffer make_commandbuffers(commandBufferInputChunk inputChunk) {
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = inputChunk.commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		for (int i = 0; i < inputChunk.frames.size(); ++i) {
			try {
				inputChunk.frames[i].commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];
			}
			catch (vk::SystemError err) {
				std::cout << "failed to allocate main command buffer\n";
			}
		}

		try {
			vk::CommandBuffer commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];
			return commandBuffer;
		}
		catch (vk::SystemError err) {
			std::cout << "Failed to allocate main command buffer\n";
			return nullptr;
		}
	}

	inline void make_framecommandbuffers(commandBufferInputChunk inputChunk) {
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.commandPool = inputChunk.commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;
		for (int i = 0; i < inputChunk.frames.size(); ++i) {
			try {
				inputChunk.frames[i].commandBuffer = inputChunk.device.allocateCommandBuffers(allocInfo)[0];
			}
			catch (vk::SystemError err) {
				std::cout << "Failed to allocate command buffer for frame " << i << std::endl;
			}
		}
	}

}