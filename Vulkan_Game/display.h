#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "device.h"
#include "scene.h"
#include "triangle_mesh.h"
#include "vertex_menagerie.h"
#include "image.h"
#include "frame.h"
#include <glm/gtc/matrix_transform.hpp>
#include "texture.h"
#include "cubemap.h"
#include "pipeline.h"
#include "util.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class Display {
public:
	Display();
	~Display();

	float girlRot = 270.0f;
	int score = 0;

	GLFWwindow* window{ nullptr };
	vk::Instance instance{ nullptr };
	vk::SurfaceKHR surface;
	vk::PhysicalDevice physicalDevice{ nullptr };
	vk::Device device{ nullptr };
	vk::Queue graphicsQueue{ nullptr };
	vk::Queue presentQueue{ nullptr };
	vk::SwapchainKHR swapchain{ nullptr };
	std::vector<frame::SwapChainFrame> swapchainFrames;
	vk::Format swapchainFormat;
	vk::Extent2D swapchainExtent;

	std::vector<pipelineType> pipelineTypes = { {pipelineType::SKY, pipelineType::STANDARD} };
	std::unordered_map<pipelineType, vk::PipelineLayout> pipelineLayout;
	std::unordered_map<pipelineType, vk::RenderPass> renderpass;
	std::unordered_map<pipelineType, vk::Pipeline> pipeline;

	std::unordered_map<pipelineType, vk::DescriptorSetLayout> frameSetLayout;
	vk::DescriptorPool frameDescriptorPool;
	std::unordered_map<pipelineType, vk::DescriptorSetLayout> meshSetLayout;
	vk::DescriptorPool meshDescriptorPool;

	vk::CommandPool commandPool;
	vk::CommandBuffer mainCommandBuffer;

	int maxFramesInFlight, frameNumber;

	VertexMenagerie* meshes;
	std::unordered_map<memory::meshTypes, image::Texture*> materials;
	image::CubeMap* cubemap;

	void render(Scene* scene);

private:
	void build_window();
	void build_instance();
	void build_device();
	void build_swapchain();
	void rebuild_swapchain();
	void build_descriptor_set_layout();
	void build_pipeline();
	void finalize_setup();
	void build_framebuffers();
	void build_frameresources();
	void build_assets();
	void prepare_frame(uint32_t imageIndex, Scene* scene);
	void prepare_scene(vk::CommandBuffer commandBuffer);
	void record_draw_commands_sky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
	void record_draw_commands_scene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
	void render_objects(vk::CommandBuffer commandBuffer, memory::meshTypes objectType, uint32_t& startInstance, uint32_t instanceCount);
	void movecharacter(Scene* scene);
	void collision(Scene* scene);
	void cleanup_swapchain();
};