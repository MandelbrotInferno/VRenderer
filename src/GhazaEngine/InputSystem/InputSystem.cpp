


#include "GhazaEngine/InputSystem/InputSystem.hpp"
#include "GhazaEngine/Logger/Logger.hpp"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>


namespace GhazaEngine
{

	void InputSystem::RegisterKeyboardInputs(const SDL_Event& l_event)
	{
		const bool* lv_keyStates = SDL_GetKeyboardState(nullptr);

		if (nullptr == lv_keyStates) {
			LOG(Level::ERROR, Category::GENERAL, "Keyboard state returned is null pointer for the following reason: %s", SDL_GetError());

			throw "Keyboard state returned is null pointer.";
		}

		memset(m_keyStates.data(), (int)KeyStates::INVALID, sizeof(KeyStates) * m_keyStates.size());
		

		if (true == lv_keyStates[SDL_SCANCODE_W]) {
			m_keyStates[(int)Keys::KEY_W] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_S]) {
			m_keyStates[(int)Keys::KEY_S] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_D]) {
			m_keyStates[(int)Keys::KEY_D] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_A]) {
			m_keyStates[(int)Keys::KEY_A] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_F1]) {
			m_keyStates[(int)Keys::KEY_F1] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_C]) {
			m_keyStates[(int)Keys::KEY_C] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_F]) {
			m_keyStates[(int)Keys::KEY_F] = KeyStates::KEY_DOWN;
		}
		if (true == lv_keyStates[SDL_SCANCODE_T]) {
			m_keyStates[(int)Keys::KEY_T] = KeyStates::KEY_DOWN;
		}

		if (SDL_EVENT_KEY_UP == l_event.type) {

			if (SDL_SCANCODE_W == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_W] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_S == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_S] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_D == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_D] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_A == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_A] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_F1 == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_F1] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_C == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_C] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_F == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_F] = KeyStates::KEY_UP;
			}
			if (SDL_SCANCODE_T == l_event.key.scancode) {
				m_keyStates[(int)Keys::KEY_T] = KeyStates::KEY_UP;
			}

		}


	}

	bool InputSystem::IsKeyDown(const Keys l_key) const
	{
		if (KeyStates::KEY_DOWN == m_keyStates[(uint32_t)l_key]) {
			return true;
		}

		return false;
	}

	void InputSystem::RegisterMouseInputs(const SDL_Event& l_event)
	{
		memset(m_mouseStatesPressed.data(), 0, sizeof(bool) * m_mouseStatesPressed.size());
		memset(m_mouseStatesUp.data(), 0, sizeof(bool) * m_mouseStatesUp.size());

		if (SDL_EVENT_MOUSE_BUTTON_DOWN == l_event.button.type) {
			
			if (SDL_BUTTON_LEFT == l_event.button.button) {
				m_mouseStatesPressed[(int)Mouse::LEFT] = true;
			}

			if (SDL_BUTTON_RIGHT == l_event.button.button) {
				m_mouseStatesPressed[(int)Mouse::RIGHT] = true;
			}
		}

		if (SDL_EVENT_MOUSE_BUTTON_UP == l_event.button.type) {
			
			if (SDL_BUTTON_LEFT == l_event.button.button) {
				m_mouseStatesUp[(int)Mouse::LEFT] = true;
			}
			if (SDL_BUTTON_RIGHT == l_event.button.button) {
				m_mouseStatesUp[(int)Mouse::RIGHT] = true;
			}
		}
	}


	void InputSystem::ProcessKeyboardInputs(SDL_Window* l_window)
	{
		
	}


	void InputSystem::ProcessMouseInputs(SDL_Window* l_window)
	{
		SDL_GetMouseState(&m_mousePosRelToWindow.x, &m_mousePosRelToWindow.y);
	}

	void InputSystem::ProcessInput(const SDL_Event& l_event, SDL_Window* l_window)
	{

		RegisterKeyboardInputs(l_event);
		RegisterMouseInputs(l_event);

		ProcessKeyboardInputs(l_window);
		ProcessMouseInputs(l_window);

	}


	bool InputSystem::IsKeyUp(const Keys l_key) const
	{
		if (KeyStates::KEY_UP == m_keyStates[(int)l_key]) {
			return true;
		}

		return false;
	}

	bool InputSystem::IsMouseButtonPressed(const Mouse l_mouseButton) const
	{
		if (true == m_mouseStatesPressed[(int)l_mouseButton]) {
			return true;
		}
		return false;
	}
	bool InputSystem::IsMouseButtonUp(const Mouse l_mouseButton) const
	{
		if (true == m_mouseStatesUp[(int)l_mouseButton]) {
			return true;
		}
		return false;
	}

	const glm::vec2& InputSystem::GetMousePosRelToWindow() const
	{
		return m_mousePosRelToWindow;
	}


	bool InputSystem::IsMouseHidden() const
	{
		return m_mouseHidden;
	}
}