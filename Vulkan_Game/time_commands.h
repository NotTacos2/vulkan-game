#pragma once
#include <vulkan/vulkan.hpp>
namespace time_commands {
	void startJob(vk::CommandBuffer commandBuffer);
	void endJob(vk::CommandBuffer commandBuffer, vk::Queue submissionQueue);
}