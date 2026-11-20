#pragma once
#include <vulkan/vulkan.hpp>
#include "shaders.h"
#include <unordered_map>

namespace pipeline {

	struct GraphicsPipelineOutBundle {
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};

	class PipelineBuilder {

	public:

		PipelineBuilder(vk::Device device);
		~PipelineBuilder();

		void reset();

		void specify_vertex_format(
			vk::VertexInputBindingDescription bindingDescription,
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions);

		void specify_vertex_shader(const char* filename);

		void specify_fragment_shader(const char* filename);

		void specify_swapchain_extent(vk::Extent2D screen_size);

		void specify_depth_attachment(const vk::Format& depthFormat, uint32_t attachment_index);

		void clear_depth_attachment();

		void add_color_attachment(const vk::Format& format, uint32_t attachment_index);

		void set_overwrite_mode(bool mode);

		GraphicsPipelineOutBundle build();

		void add_descriptor_set_layout(vk::DescriptorSetLayout descriptorSetLayout);

		void reset_descriptor_set_layouts();

	private:
		vk::Device device;
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};

		vk::VertexInputBindingDescription bindingDescription;
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		vk::ShaderModule vertexShader = nullptr, fragmentShader = nullptr;
		vk::PipelineShaderStageCreateInfo vertexShaderInfo, fragmentShaderInfo;

		vk::Extent2D swapchainExtent;
		vk::Viewport viewport = {};
		vk::Rect2D scissor = {};
		vk::PipelineViewportStateCreateInfo viewportState = {};

		vk::PipelineRasterizationStateCreateInfo rasterizer = {};

		vk::PipelineDepthStencilStateCreateInfo depthState;
		std::unordered_map<uint32_t, vk::AttachmentDescription> attachmentDescriptions;
		std::unordered_map<uint32_t, vk::AttachmentReference> attachmentReferences;
		std::vector<vk::AttachmentDescription> flattenedAttachmentDescriptions;
		std::vector<vk::AttachmentReference> flattenedAttachmentReferences;

		vk::PipelineMultisampleStateCreateInfo multisampling = {};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		vk::PipelineColorBlendStateCreateInfo colorBlending = {};

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		bool overwrite;

		void reset_vertex_format();

		void reset_shader_modules();

		void reset_renderpass_attachments();

		vk::AttachmentDescription make_renderpass_attachment(
			const vk::Format& format, vk::AttachmentLoadOp loadOp,
			vk::AttachmentStoreOp storeOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);

		vk::AttachmentReference make_attachment_reference(
			uint32_t attachment_index, vk::ImageLayout layout);

		void configure_input_assembly();

		vk::PipelineShaderStageCreateInfo make_shader_info(
			const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage);

		vk::PipelineViewportStateCreateInfo make_viewport_state();

		void make_rasterizer_info();

		void configure_multisampling();

		void configure_color_blending();

		vk::PipelineLayout make_pipeline_layout();

		vk::RenderPass make_renderpass();

		vk::SubpassDescription make_subpass(
			const std::vector<vk::AttachmentReference>& attachments
		);

		vk::RenderPassCreateInfo make_renderpass_info(
			const std::vector<vk::AttachmentDescription>& attachments,
			const vk::SubpassDescription& subpass
		);
	};
}