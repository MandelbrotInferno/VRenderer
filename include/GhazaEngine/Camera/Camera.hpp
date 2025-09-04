#pragma once


#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>



namespace GhazaEngine
{
	class InputSystem;

	class Camera final
	{
    public:

        glm::mat4 GetViewMatrix() const;

        void Update(const double l_deltaTime, const InputSystem& l_inputSystem);

		glm::vec3 GetPosition() const;


	private:

		void SetUpVector(const glm::vec3& l_up);

    private:

		struct Movement
		{
		public:

			bool m_forward = false;
			bool m_backward = false;
			bool m_left = false;
			bool m_right = false;
			bool m_up = false;
			bool m_down = false;
			bool m_fastSpeed = false;
		};

	private:
		float m_mouseSpeed = 4.0f;
		float m_acceleration = 150.0f;
		float m_damping = 0.2f;
		float m_maxSpeed = 10.0f;
		float m_fastCoef = 10.0f;

		glm::vec3 m_cameraPosition{};
		glm::quat m_cameraOrientation = glm::quat(glm::vec3(0));
		glm::vec3 m_moveSpeed{};
		glm::vec2 m_prevMousePosRelToWindow{};
		glm::vec3 m_up = glm::vec3(0.0f, 0.0f, 1.0f);
		Movement m_movement{};
	};

}