#pragma once




#include <glm/glm.hpp>



namespace VRenderer
{
	struct GPUUniformBufferData final
	{
    public:

        glm::mat4 m_view;
        glm::mat4 m_proj;
        glm::mat4 m_viewproj;
        glm::vec4 m_ambientColor;
        glm::vec4 m_sunlightDirection; // w for sun power
        glm::vec4 m_sunlightColor;
	};
}