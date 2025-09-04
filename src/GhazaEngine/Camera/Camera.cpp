



#include "GhazaEngine/Camera/Camera.hpp"
#include "GhazaEngine/InputSystem/InputSystem.hpp"

namespace GhazaEngine
{
	glm::mat4 Camera::GetViewMatrix() const
	{
		const glm::mat4 lv_t = glm::translate(glm::mat4(1.0f), -m_cameraPosition);
		const glm::mat4 lv_r = glm::mat4_cast(m_cameraOrientation);
		return lv_r * lv_t;
	}

	void Camera::Update(const double l_deltaTime, const InputSystem& l_inputSystem)
	{
		const bool lv_leftMousePressed = l_inputSystem.IsMouseButtonPressed(InputSystem::Mouse::LEFT);
		const glm::vec2 lv_currentMousePosRelToWindow = l_inputSystem.GetMousePosRelToWindow();
		if (true == lv_leftMousePressed)
		{
			const glm::vec2 delta = lv_currentMousePosRelToWindow - m_prevMousePosRelToWindow;
			const glm::quat deltaQuat = glm::quat(glm::vec3(m_mouseSpeed * delta.y, m_mouseSpeed * delta.x, 0.0f));
			m_cameraOrientation = deltaQuat * m_cameraOrientation;
			m_cameraOrientation = glm::normalize(m_cameraOrientation);
			SetUpVector(m_up);
		}
		m_prevMousePosRelToWindow = lv_currentMousePosRelToWindow;

		const glm::mat4 lv_v = glm::mat4_cast(m_cameraOrientation);

		const glm::vec3 lv_forward = -glm::vec3(lv_v[0][2], lv_v[1][2], lv_v[2][2]);
		const glm::vec3 lv_right = glm::vec3(lv_v[0][0], lv_v[1][0], lv_v[2][0]);
		const glm::vec3 lv_up = glm::cross(lv_right, lv_forward);

		glm::vec3 lv_accel(0.0f);

		if (m_movement.m_forward) lv_accel += lv_forward;
		if (m_movement.m_backward) lv_accel -= lv_forward;

		if (m_movement.m_left) lv_accel -= lv_right;
		if (m_movement.m_right) lv_accel += lv_right;

		if (m_movement.m_up) lv_accel += m_up;
		if (m_movement.m_down) lv_accel -= m_up;

		if (m_movement.m_fastSpeed) lv_accel *= m_fastCoef;

		if (glm::vec3(0) == lv_accel)
		{
			// decelerate naturally according to the damping value
			m_moveSpeed -= m_moveSpeed * std::min((1.0f / m_damping) * static_cast<float>(l_deltaTime), 1.0f);
		}
		else
		{
			// acceleration
			m_moveSpeed += lv_accel * m_acceleration * static_cast<float>(l_deltaTime);
			const float lv_maxSpeed = m_movement.m_fastSpeed ? m_maxSpeed * m_fastCoef : m_maxSpeed;
			if (glm::length(m_moveSpeed) > lv_maxSpeed) m_moveSpeed = glm::normalize(m_moveSpeed) * lv_maxSpeed;
		}

		m_cameraPosition += m_moveSpeed * static_cast<float>(l_deltaTime);
	}

	glm::vec3 Camera::GetPosition() const
	{
		return m_cameraPosition;
	}

	void Camera::SetUpVector(const glm::vec3& l_up)
	{
		const glm::mat4 lv_view = GetViewMatrix();
		const glm::vec3 lv_dir = -glm::vec3(lv_view[0][2], lv_view[1][2], lv_view[2][2]);
		m_cameraOrientation = glm::lookAt(m_cameraPosition, m_cameraPosition + lv_dir, m_up);
	}
}