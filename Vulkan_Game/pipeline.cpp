#include "pipeline.h"

pipeline::PipelineBuilder::PipelineBuilder(vk::Device device) {
	this->device = device;
	reset();

	configure_input_assembly();
	make_rasterizer_info();
	configure_multisampling();
	configure_color_blending();
	pipelineInfo.basePipelineHandle = nullptr;
}

pipeline::PipelineBuilder::~PipelineBuilder() {
	reset();
}

void pipeline::PipelineBuilder::reset() {

	pipelineInfo.flags = vk::PipelineCreateFlags();

	reset_vertex_format();
	reset_shader_modules();
	reset_renderpass_attachments();
	reset_descriptor_set_layouts();
}

void pipeline::PipelineBuilder::reset_vertex_format() {

	vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

}

void pipeline::PipelineBuilder::reset_renderpass_attachments() {
	attachmentDescriptions.clear();
	attachmentReferences.clear();
}

void pipeline::PipelineBuilder::specify_vertex_format(
	vk::VertexInputBindingDescription bindingDescription,
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions) {

	this->bindingDescription = bindingDescription;
	this->attributeDescriptions = attributeDescriptions;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &(this->bindingDescription);
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(this->attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = this->attributeDescriptions.data();

}

void pipeline::PipelineBuilder::set_overwrite_mode(bool mode) {
	overwrite = mode;
}

void pipeline::PipelineBuilder::reset_shader_modules() {
	if (vertexShader) {
		device.destroyShaderModule(vertexShader);
		vertexShader = nullptr;
	}
	if (fragmentShader) {
		device.destroyShaderModule(fragmentShader);
		fragmentShader = nullptr;
	}
	shaderStages.clear();
}

void pipeline::PipelineBuilder::specify_vertex_shader(const char* filename) {

	if (vertexShader) {
		device.destroyShaderModule(vertexShader);
		vertexShader = nullptr;
	}
	vertexShader = shader::createModule(filename, device);
	vertexShaderInfo = make_shader_info(vertexShader, vk::ShaderStageFlagBits::eVertex);
	shaderStages.push_back(vertexShaderInfo);
}

void pipeline::PipelineBuilder::specify_fragment_shader(const char* filename) {

	if (fragmentShader) {
		device.destroyShaderModule(fragmentShader);
		fragmentShader = nullptr;
	}
	fragmentShader = shader::createModule(filename, device);
	fragmentShaderInfo = make_shader_info(fragmentShader, vk::ShaderStageFlagBits::eFragment);
	shaderStages.push_back(fragmentShaderInfo);
}

vk::PipelineShaderStageCreateInfo pipeline::PipelineBuilder::make_shader_info(
	const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage) {

	vk::PipelineShaderStageCreateInfo shaderInfo = {};
	shaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	shaderInfo.stage = stage;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
	return shaderInfo;
}

void pipeline::PipelineBuilder::specify_swapchain_extent(vk::Extent2D screen_size) {

	swapchainExtent = screen_size;
}

void pipeline::PipelineBuilder::specify_depth_attachment(
	const vk::Format& depthFormat, uint32_t attachment_index) {

	depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
	depthState.depthTestEnable = true;
	depthState.depthWriteEnable = true;
	depthState.depthCompareOp = vk::CompareOp::eLess;
	depthState.depthBoundsTestEnable = false;
	depthState.stencilTestEnable = false;

	pipelineInfo.pDepthStencilState = &depthState;
	attachmentDescriptions.insert(
		{ attachment_index,
		make_renderpass_attachment(
			depthFormat, vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal)
		}
	);
	attachmentReferences.insert(
		{ attachment_index,
		make_attachment_reference(attachment_index, vk::ImageLayout::eDepthStencilAttachmentOptimal)
		}
	);
}

void pipeline::PipelineBuilder::add_color_attachment(
	const vk::Format& format, uint32_t attachment_index) {

	vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eDontCare;
	if (overwrite) {
		loadOp = vk::AttachmentLoadOp::eLoad;
	}
	vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

	vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
	if (overwrite) {
		initialLayout = vk::ImageLayout::ePresentSrcKHR;
	}
	vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR;

	attachmentDescriptions.insert(
		{ attachment_index,
		make_renderpass_attachment(format, loadOp, storeOp, initialLayout, finalLayout)
		}
	);
	attachmentReferences.insert(
		{ attachment_index,
		make_attachment_reference(attachment_index, vk::ImageLayout::eColorAttachmentOptimal)
		}
	);
}

vk::AttachmentDescription pipeline::PipelineBuilder::make_renderpass_attachment(
	const vk::Format& format, vk::AttachmentLoadOp loadOp,
	vk::AttachmentStoreOp storeOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout) {

	vk::AttachmentDescription attachment = {};
	attachment.flags = vk::AttachmentDescriptionFlags();
	attachment.format = format;
	attachment.samples = vk::SampleCountFlagBits::e1;
	attachment.loadOp = loadOp;
	attachment.storeOp = storeOp;
	attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachment.initialLayout = initialLayout;
	attachment.finalLayout = finalLayout;

	return attachment;
}

vk::AttachmentReference pipeline::PipelineBuilder::make_attachment_reference(
	uint32_t attachment_index, vk::ImageLayout layout) {

	vk::AttachmentReference attachmentRef = {};
	attachmentRef.attachment = attachment_index;
	attachmentRef.layout = layout;

	return attachmentRef;
}

void pipeline::PipelineBuilder::clear_depth_attachment() {
	pipelineInfo.pDepthStencilState = nullptr;
}

void pipeline::PipelineBuilder::add_descriptor_set_layout(vk::DescriptorSetLayout descriptorSetLayout) {
	descriptorSetLayouts.push_back(descriptorSetLayout);
}

void pipeline::PipelineBuilder::reset_descriptor_set_layouts() {
	descriptorSetLayouts.clear();
}

pipeline::GraphicsPipelineOutBundle pipeline::PipelineBuilder::build() {
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	make_viewport_state();
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	vk::PipelineLayout pipelineLayout = make_pipeline_layout();
	pipelineInfo.layout = pipelineLayout;

	vk::RenderPass renderpass = make_renderpass();
	pipelineInfo.renderPass = renderpass;
	pipelineInfo.subpass = 0;

	vk::Pipeline graphicsPipeline;
	try {
		graphicsPipeline = (device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
	}
	catch (vk::SystemError err) {
		std::cout << "Failed to create Pipeline\n";
	}

	GraphicsPipelineOutBundle output;
	output.layout = pipelineLayout;
	output.renderpass = renderpass;
	output.pipeline = graphicsPipeline;

	return output;
}

void pipeline::PipelineBuilder::configure_input_assembly() {

	inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

}

vk::PipelineViewportStateCreateInfo pipeline::PipelineBuilder::make_viewport_state() {

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchainExtent.width;
	viewport.height = (float)swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x = 0.0f;
	scissor.offset.y = 0.0f;
	scissor.extent = swapchainExtent;

	viewportState.flags = vk::PipelineViewportStateCreateFlags();
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	return viewportState;
}

void pipeline::PipelineBuilder::make_rasterizer_info() {

	rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;

}

void pipeline::PipelineBuilder::configure_multisampling() {

	multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

}

void pipeline::PipelineBuilder::configure_color_blending() {

	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

}

vk::PipelineLayout pipeline::PipelineBuilder::make_pipeline_layout() {
	vk::PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.flags = vk::PipelineLayoutCreateFlags();

	layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pSetLayouts = descriptorSetLayouts.data();

	layoutInfo.pushConstantRangeCount = 0;

	try {
		return device.createPipelineLayout(layoutInfo);
	}
	catch (vk::SystemError err) {
		std::cout << "Failed to create pipeline layout!";
	}
}

vk::RenderPass pipeline::PipelineBuilder::make_renderpass() {

	flattenedAttachmentDescriptions.clear();
	flattenedAttachmentReferences.clear();
	size_t attachmentCount = attachmentDescriptions.size();
	flattenedAttachmentDescriptions.resize(attachmentCount);
	flattenedAttachmentReferences.resize(attachmentCount);

	for (int i = 0; i < attachmentCount; ++i) {
		flattenedAttachmentDescriptions[i] = attachmentDescriptions[i];
		flattenedAttachmentReferences[i] = attachmentReferences[i];
	}

	vk::SubpassDescription subpass = make_subpass(flattenedAttachmentReferences);
	vk::RenderPassCreateInfo renderpassInfo = make_renderpass_info(flattenedAttachmentDescriptions, subpass);
	try {
		return device.createRenderPass(renderpassInfo);
	}
	catch (vk::SystemError err) {
		std::cout << "Failed to create renderpass!";
	}

}

vk::SubpassDescription pipeline::PipelineBuilder::make_subpass(
	const std::vector<vk::AttachmentReference>& attachments) {

	vk::SubpassDescription subpass = {};
	subpass.flags = vk::SubpassDescriptionFlags();
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachments[0];
	if (attachments.size() > 1) {
		subpass.pDepthStencilAttachment = &attachments[1];
	}
	else {
		subpass.pDepthStencilAttachment = nullptr;
	}

	return subpass;
}

vk::RenderPassCreateInfo pipeline::PipelineBuilder::make_renderpass_info(
	const std::vector<vk::AttachmentDescription>& attachments,
	const vk::SubpassDescription& subpass) {

	vk::RenderPassCreateInfo renderpassInfo = {};
	renderpassInfo.flags = vk::RenderPassCreateFlags();
	renderpassInfo.attachmentCount = attachments.size();
	renderpassInfo.pAttachments = attachments.data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;

	return renderpassInfo;
}