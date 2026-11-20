#include "display.h"
#include <iostream>
#include "instance.h"
#include <vulkan/vulkan.hpp>
#include "device.h"
#include <GLFW/glfw3.h>
#include "shaders.h"
#include <array>
#include "scene.h"
#include "descriptors.h"
#include "frame.h"
#include "obj_mesh.h"

Display::Display() {
	build_window();
	build_instance();
	build_device();
	build_descriptor_set_layout();
	build_pipeline();
	finalize_setup();
	build_assets();
}

void Display::build_window() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(800, 600, "Vulkan Game", nullptr, nullptr);
	if (!window) {
		std::cout << "Failed to create GLFW window!\n";
		glfwTerminate();
		return;
	}
}

void Display::build_instance() {
	instance = instance::make_instance("Vulkan Game");
	VkSurfaceKHR stylesurface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &stylesurface) != VK_SUCCESS) {
		std::cout << "failed to create glfw surface for vulkan\n";
	}
	else {
		
	}
	surface = stylesurface;
}

void Display::build_device() {
	physicalDevice = device::choosedevice(instance);
	device = device::create_logical_device(physicalDevice, surface);
	std::array<vk::Queue, 2> queues = device::get_queue(physicalDevice, device, surface);
	graphicsQueue = queues[0];
	presentQueue = queues[1];
	build_swapchain();
	frameNumber = 0;
}

void Display::build_swapchain() {
	device::SwapChainBundle bundle = device::create_swapchain(device, physicalDevice, surface, 800, 600);
	swapchain = bundle.swapchain;
	swapchainFrames = bundle.frames;
	swapchainFormat = bundle.format;
	swapchainExtent = bundle.extent;
	maxFramesInFlight = static_cast<int>(swapchainFrames.size());

	for (frame::SwapChainFrame& frame : swapchainFrames) {
		frame.logicalDevice = device;
		frame.physicalDevice = physicalDevice;
		frame.width = swapchainExtent.width;
		frame.height = swapchainExtent.height;

		frame.build_depth_resources();
	}
}

void Display::rebuild_swapchain() {
	int width = 0;
	int height = 0;
	while (width == 0 || height == 0) {
		width = 800;
		height = 600;
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	cleanup_swapchain();
	build_swapchain();
	build_framebuffers();
	build_frameresources();
	device::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	device::make_framecommandbuffers(commandBufferInput);
}

void Display::build_descriptor_set_layout() {
	descriptors::descriptorSetLayoutData bindings;
	bindings.count = 1;

	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameSetLayout[pipelineType::SKY] = descriptors::build_descriptorsetlayout(device, bindings);

	bindings.count = 2;

	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameSetLayout[pipelineType::STANDARD] = descriptors::build_descriptorsetlayout(device, bindings);

	bindings.count = 1;

	bindings.indices[0] = 0;
	bindings.types[0] = vk::DescriptorType::eCombinedImageSampler;
	bindings.counts[0] = 1;
	bindings.stages[0] = vk::ShaderStageFlagBits::eFragment;

	meshSetLayout[pipelineType::SKY] = descriptors::build_descriptorsetlayout(device, bindings);
	meshSetLayout[pipelineType::STANDARD] = descriptors::build_descriptorsetlayout(device, bindings);
}

void Display::build_pipeline() {
	pipeline::PipelineBuilder pipelineBuilder(device);

	pipelineBuilder.set_overwrite_mode(false);
	pipelineBuilder.specify_vertex_shader("assets/sky_vertex.spv");
	pipelineBuilder.specify_fragment_shader("assets/sky_fragment.spv");
	pipelineBuilder.specify_swapchain_extent(swapchainExtent);
	pipelineBuilder.clear_depth_attachment();
	pipelineBuilder.add_descriptor_set_layout(frameSetLayout[pipelineType::SKY]);
	pipelineBuilder.add_descriptor_set_layout(meshSetLayout[pipelineType::SKY]);
	pipelineBuilder.add_color_attachment(swapchainFormat, 0);

	pipeline::GraphicsPipelineOutBundle output = pipelineBuilder.build();

	pipelineLayout[pipelineType::SKY] = output.layout;
	renderpass[pipelineType::SKY] = output.renderpass;
	pipeline[pipelineType::SKY] = output.pipeline;
	pipelineBuilder.reset();

	pipelineBuilder.set_overwrite_mode(true);
	pipelineBuilder.specify_vertex_format(device::getPosColorBindingDescription(), device::getPosColorAttributeDesciptions());
	pipelineBuilder.specify_vertex_shader("assets/vertex.spv");
	pipelineBuilder.specify_fragment_shader("assets/fragment.spv");
	pipelineBuilder.specify_swapchain_extent(swapchainExtent);
	pipelineBuilder.specify_depth_attachment(swapchainFrames[0].depthFormat, 1);
	pipelineBuilder.add_descriptor_set_layout(frameSetLayout[pipelineType::STANDARD]);
	pipelineBuilder.add_descriptor_set_layout(meshSetLayout[pipelineType::STANDARD]);
	pipelineBuilder.add_color_attachment(swapchainFormat, 0);

	output = pipelineBuilder.build();

	pipelineLayout[pipelineType::STANDARD] = output.layout;
	renderpass[pipelineType::STANDARD] = output.renderpass;
	pipeline[pipelineType::STANDARD] = output.pipeline;
}

void Display::build_framebuffers() {
	device::framebufferInput frameBufferInput;
	frameBufferInput.device = device;
	frameBufferInput.renderpass = renderpass;
	frameBufferInput.swapchainExtent = swapchainExtent;
	device::make_framebuffers(frameBufferInput, swapchainFrames);
}

void Display::build_frameresources() {
	descriptors::descriptorSetLayoutData bindings;
	bindings.count = 2;
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	uint32_t descriptor_sets_per_frame = 2;

	frameDescriptorPool = descriptors::build_descriptorpool(device, static_cast<uint32_t>(swapchainFrames.size() * descriptor_sets_per_frame), bindings);

	for (frame::SwapChainFrame& frame : swapchainFrames) {
		frame.imageAvailable = device::make_semaphore(device);
		frame.renderFinished = device::make_semaphore(device);
		frame.inFlight = device::make_fence(device);

		frame.build_descriptor_resources();
		frame.descriptorSet[pipelineType::SKY] = descriptors::allocate_descriptor_set(device, frameDescriptorPool, frameSetLayout[pipelineType::SKY]);
		frame.descriptorSet[pipelineType::STANDARD] = descriptors::allocate_descriptor_set(device, frameDescriptorPool, frameSetLayout[pipelineType::STANDARD]);

		frame.record_write_operations();
	}
}

void Display::finalize_setup() {
	build_framebuffers();
	commandPool = device::make_commandpool(device, physicalDevice, surface);
	device::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	mainCommandBuffer = device::make_commandbuffers(commandBufferInput);
	device::make_framecommandbuffers(commandBufferInput);
	build_frameresources();
}

void Display::build_assets() {
	meshes = new VertexMenagerie();
	std::unordered_map<memory::meshTypes, std::vector<const char*>> model_filenames = {
		{memory::meshTypes::GROUND, {"assets/ground.obj","assets/ground.mtl"}},
		{memory::meshTypes::GIRL, {"assets/girl.obj","assets/girl.mtl"}},
		{memory::meshTypes::CUBE, {"assets/cube.obj","assets/cube.mtl"}},
	};
	std::unordered_map<memory::meshTypes, glm::mat4> preTransforms = {
		{memory::meshTypes::GROUND, glm::mat4(1.0f)},
		{memory::meshTypes::GIRL, glm::rotate(
			glm::mat4(1.0f),
			glm::radians(90.0f),
			glm::vec3(1.0f, 0.0f, 0.0f)
		)},
		{memory::meshTypes::CUBE, glm::mat4(1.0f)}
	};

	for (std::pair<memory::meshTypes, std::vector<const char*>> pair : model_filenames) {
		objmesh::ObjMesh model(pair.second[0], pair.second[1], preTransforms[pair.first]);
		meshes->consume(pair.first, model.vertices, model.indices);
	}

	vertexBufferFinalizationChunk finalizationInfo;
	finalizationInfo.logicalDevice = device;
	finalizationInfo.physicalDevice = physicalDevice;
	finalizationInfo.commandBuffer = mainCommandBuffer;
	finalizationInfo.queue = graphicsQueue;
	meshes->finalize(finalizationInfo);

	std::unordered_map<memory::meshTypes, std::vector<const char*>> filenames = {
		{memory::meshTypes::GROUND, {"assets/ground.jpg"}},
		{memory::meshTypes::GIRL, {"assets/none.png"}},
		{memory::meshTypes::CUBE, {"assets/none.png"}}
	};

	descriptors::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);

	meshDescriptorPool = descriptors::build_descriptorpool(device, static_cast<uint32_t>(filenames.size()) + 1, bindings);

	image::TextureInputChunk textureInfo;
	textureInfo.commandBuffer = mainCommandBuffer;
	textureInfo.queue = graphicsQueue;
	textureInfo.logicalDevice = device;
	textureInfo.physicalDevice = physicalDevice;
	textureInfo.layout = meshSetLayout[pipelineType::STANDARD];
	textureInfo.descriptorPool = meshDescriptorPool;

	for (const auto& [object, filename] : filenames) {
		textureInfo.filenames = filename;
		materials[object] = new image::Texture(textureInfo);
	}

	textureInfo.layout = meshSetLayout[pipelineType::SKY];
	textureInfo.descriptorPool = meshDescriptorPool;
	textureInfo.filenames = { {
			"assets/sky_front.png",
			"assets/sky_back.png",
			"assets/sky_left.png",
			"assets/sky_right.png",
			"assets/sky_bottom.png",
			"assets/sky_top.png",
	} };
	cubemap = new image::CubeMap(textureInfo);
}

void Display::prepare_frame(uint32_t imageIndex, Scene* scene) {
	frame::SwapChainFrame& _frame = swapchainFrames[imageIndex];
	glm::vec4 cam_vec_forwards = { 1.0f,  0.0f, 0.0f, 0.0f };
	glm::vec4 cam_vec_right = { 0.0f, -1.0f, 0.0f, 0.0f };
	glm::vec4 cam_vec_up = { 0.0f,  0.0f, 1.0f, 0.0f };
	_frame.cameraVectorData.forwards = cam_vec_forwards;
	_frame.cameraVectorData.right = cam_vec_right;
	_frame.cameraVectorData.up = cam_vec_up;
	memcpy(_frame.cameraVectorWriteLocation, &(_frame.cameraVectorData), sizeof(frame::CameraVectors));

	glm::vec3 eye = { 0.5f, 0.0f, 2.0f };
	glm::vec3 center = { 1.0f, 0.0f, 2.0f };
	glm::vec3 up = { 0.0f, 0.0f, 1.0f };
	glm::mat4 view = glm::lookAt(eye, center, up);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height), 0.1f, 100.0f);
	projection[1][1] *= -1;

	_frame.cameraMatrixData.view = view;
	_frame.cameraMatrixData.projection = projection;
	_frame.cameraMatrixData.viewProjection = projection * view;
	memcpy(_frame.cameraMatrixWriteLocation, &(_frame.cameraMatrixData), sizeof(frame::CameraMatrices));

	size_t i = 0;
	for (std::pair<memory::meshTypes, std::vector<glm::vec3>> pair : scene->positions) {
		for (glm::vec3& position : pair.second) {
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
			if (pair.first == memory::meshTypes::GIRL)
			{
				model = glm::rotate(
					model,
					glm::radians(girlRot),
					glm::vec3(0.0f, 0.0f, 1.0f)
				);
			}
			if (pair.first == memory::meshTypes::CUBE)
			{
				model = glm::scale(
					model,
					glm::vec3(0.25f, 0.25f, 0.25f)
				);
			}
			_frame.modelTransforms[i++] = model;
		}
	}
	memcpy(_frame.modelBufferWriteLocation, _frame.modelTransforms.data(), i * sizeof(glm::mat4));
	_frame.write_descriptor_set();
}

void Display::prepare_scene(vk::CommandBuffer commandBuffer) {
	vk::Buffer vertexBuffers[] = { meshes->vertexBuffer.buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(meshes->indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

void Display::record_draw_commands_sky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass[pipelineType::SKY];
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer[pipelineType::SKY];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchainExtent;

	vk::ClearValue colorClear;
	std::array<float, 4> colors = { 1.0f, 0.5f, 0.25f, 1.0f };

	std::vector<vk::ClearValue> clearValues = { {colorClear} };

	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline[pipelineType::SKY]);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[pipelineType::SKY], 0, swapchainFrames[imageIndex].descriptorSet[pipelineType::SKY], nullptr);

	cubemap->use(commandBuffer, pipelineLayout[pipelineType::SKY]);
	commandBuffer.draw(6, 1, 0, 0);

	commandBuffer.endRenderPass();
}

void Display::record_draw_commands_scene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) {

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass[pipelineType::STANDARD];
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer[pipelineType::STANDARD];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchainExtent;

	vk::ClearValue colorClear;
	std::array<float, 4> colors = { 1.0f, 0.5f, 0.25f, 1.0f };
	colorClear.color = vk::ClearColorValue(colors);
	vk::ClearValue depthClear;

	depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
	std::vector<vk::ClearValue> clearValues = { {colorClear,depthClear} };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline[pipelineType::STANDARD]);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[pipelineType::STANDARD], 0, swapchainFrames[imageIndex].descriptorSet[pipelineType::STANDARD], nullptr);

	prepare_scene(commandBuffer);

	uint32_t startInstance = 0;
	for (std::pair<memory::meshTypes, std::vector<glm::vec3>> pair : scene->positions) {
		render_objects(
			commandBuffer, pair.first, startInstance, static_cast<uint32_t>(pair.second.size())
		);
	}

	commandBuffer.endRenderPass();
}

void Display::render_objects(vk::CommandBuffer commandBuffer, memory::meshTypes objectType, uint32_t& startInstance, uint32_t instanceCount) {
	int indexCount = meshes->indexCounts.find(objectType)->second;
	int firstIndex = meshes->firstIndices.find(objectType)->second;
	materials[objectType]->use(commandBuffer, pipelineLayout[pipelineType::STANDARD]);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, 0, startInstance);
	startInstance += instanceCount;
}

void Display::movecharacter(Scene* scene) {
	glm::vec3& girlPos = scene->positions[memory::meshTypes::GIRL][0];
	float speed = 0.0003f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		girlPos.x += speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		girlPos.y += speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		girlPos.x -= speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		girlPos.y -= speed;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		girlPos.x = 6;
		girlPos.y = 0;
		girlRot = 270.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		girlRot += 0.03f;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		girlRot -= 0.03f;
	
}

void Display::collision(Scene* scene) {
	glm::vec3& girlPos =
		scene->positions[memory::meshTypes::GIRL][0];

	glm::vec3& cubePos =
		scene->positions[memory::meshTypes::CUBE][0];

	float distance = glm::length(girlPos - cubePos);

	if (distance < 0.4f)
	{
		cubePos.x = (rand() % 20) / 10.0f + 5.0f;
		cubePos.y = (rand() % 20) / 10.0f - 1.0f;
		score += 1;
		std::string title = "Cube Collector - Score: " + std::to_string(score);
		glfwSetWindowTitle(window, title.c_str());
	}
}


void Display::render(Scene* scene) {
	device.waitForFences(1, &(swapchainFrames[frameNumber].inFlight), VK_TRUE, UINT64_MAX);
	device.resetFences(1, &(swapchainFrames[frameNumber].inFlight));
	uint32_t imageIndex;
	try {
		vk::ResultValue acquire = device.acquireNextImageKHR(
			swapchain,
			UINT64_MAX,
			swapchainFrames[frameNumber].imageAvailable,
			nullptr
		);
		imageIndex = acquire.value;
	}
	catch (vk::OutOfDateKHRError error) {
		rebuild_swapchain();
		return;
	}
	catch (vk::IncompatibleDisplayKHRError error) {
		rebuild_swapchain();
		return;
	}
	catch (vk::SystemError err) {
		std::cout << "Failed to create swapchain image\n";
	}

	vk::CommandBuffer commandBuffer = swapchainFrames[imageIndex].commandBuffer;
	commandBuffer.reset();
	prepare_frame(imageIndex, scene);
	vk::CommandBufferBeginInfo beginInfo = {};

	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		std::cout << "Failed to begin recording command buffer!";
	}

	record_draw_commands_sky(commandBuffer, imageIndex, scene);
	record_draw_commands_scene(commandBuffer, imageIndex, scene);
	movecharacter(scene);
	collision(scene);

	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {

		std::cout << "failed to record command buffer!";
	}
	vk::SubmitInfo submitInfo = {};
	vk::Semaphore waitSemaphores[] = { swapchainFrames[frameNumber].imageAvailable };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vk::Semaphore signalSemaphores[] = { swapchainFrames[frameNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	try {
		graphicsQueue.submit(submitInfo, swapchainFrames[frameNumber].inFlight);
	}
	catch (vk::SystemError err) {
		std::cout << "failed to submit draw command buffer\n";
	}

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	vk::Result present;
	try {
		present = presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error) {
		present = vk::Result::eErrorOutOfDateKHR;
	}

	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR) {
		rebuild_swapchain();
		return;
	}
	frameNumber = (frameNumber + 1) % maxFramesInFlight;
}

void Display::cleanup_swapchain() {
	for (frame::SwapChainFrame frame : swapchainFrames) {
		frame.destroy();
	}
	device.destroySwapchainKHR(swapchain);
	device.destroyDescriptorPool(frameDescriptorPool);
}

Display::~Display() {
	device.waitIdle();

	device.destroyCommandPool(commandPool);
	for (pipelineType pipeline_type : pipelineTypes) {
		device.destroyPipeline(pipeline[pipeline_type]);
		device.destroyPipelineLayout(pipelineLayout[pipeline_type]);
		device.destroyRenderPass(renderpass[pipeline_type]);
	}
	cleanup_swapchain();

	for (pipelineType pipeline_type : pipelineTypes) {
		device.destroyDescriptorSetLayout(frameSetLayout[pipeline_type]);
		device.destroyDescriptorSetLayout(meshSetLayout[pipeline_type]);
	}
	device.destroyDescriptorPool(meshDescriptorPool);

	delete meshes;

	for (const auto& [key, texture] : materials) {
		delete texture;
	}
	delete cubemap;

	device.destroy();

	instance.destroySurfaceKHR(surface);
	instance.destroy();
	glfwTerminate();
}