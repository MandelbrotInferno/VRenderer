#pragma once



#include <vector>
#include <array>


namespace VRenderer
{
	namespace VulkanUtils
	{
		struct VulkanGraphicsCreateInfo final
		{
			std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageCreateInfos{};
			VkPrimitiveTopology m_topology{};
			VkPolygonMode m_polygonMode{};
			VkCullModeFlags m_cullMode{};
			VkFrontFace m_frontFace{};
			VkCompareOp m_depthCompareOp;
			float m_lineWidth{};
			VkSampleCountFlagBits m_rasterizationSamples{};
			VkBool32 m_sampleShadingEnabled{};
			float m_minSampleShading{};
			VkBool32 m_depthTestEnabled{};
			VkBool32 m_depthWriteEnabled{};
			VkBool32 m_colorBlendCreateInfoLogicOpEnabled{};
			VkLogicOp m_colorBlendCreateInfoLogicOp{};
			std::vector<VkPipelineColorBlendAttachmentState> m_blendAttachmentStates{};
			std::array<float, 4> m_blendConstants{};
			std::vector<VkDynamicState> m_dynamicStates{};
			VkPipelineLayout m_pipelineLayout{};
			std::vector<VkFormat> m_colorAttachmentFormats{};
			VkFormat m_depthAttachmentFormat{};
		};
	}
}