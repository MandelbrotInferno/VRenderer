#pragma once



#include <glm/glm.hpp>



namespace VRenderer
{
	struct ComputePassPushConstant final
	{
		glm::vec4 m_data1{};
		glm::vec4 m_data2{};
		glm::vec4 m_data3{};
		glm::vec4 m_data4{};
	};
}