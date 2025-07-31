#pragma once



#include "VRenderer/Passes/ComputePasses/ComputePassPushConstant.hpp"
#include <volk.h>


namespace VRenderer
{
	struct ComputePass final
	{
		//Does not own pipeline
		VkPipeline m_pipeline{};

		const char* m_passName{};
		ComputePassPushConstant m_pushConstData{};
	};
}