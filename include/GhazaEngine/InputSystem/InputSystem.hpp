#pragma once




#include <glm/glm.hpp>
#include <array>
#include <SDL3/SDL_events.h>


namespace GhazaEngine
{

	class InputSystem final
	{

	public:

		enum class Keys : uint32_t
		{
			KEY_W = 0,
			KEY_A = 1,
			KEY_S = 2,
			KEY_D = 3,
			KEY_F1 = 4,
			KEY_C = 5,
			KEY_F = 6,
			KEY_T = 7,


			//Maximum number of possible keys. 
			// Not to be used as index into key states.
			KEY_MAXIMUM = 1024
		};

		enum class Mouse
		{
			LEFT = 0,
			RIGHT = 1
		};

		enum class KeyStates : uint32_t
		{
			KEY_UP,
			KEY_DOWN,

			INVALID
		};

	public:

		void ProcessInput(const SDL_Event& l_event, SDL_Window* l_window);
		bool IsKeyUp(const Keys l_key) const;
		bool IsKeyDown(const Keys l_key) const;

		bool IsMouseButtonPressed(const Mouse l_mouseButton) const;
		bool IsMouseButtonUp(const Mouse l_mouseButton) const;

		const glm::vec2& GetMousePosRelToWindow() const;

		bool IsMouseHidden() const;

	private:

		void RegisterKeyboardInputs(const SDL_Event& l_event);
		void RegisterMouseInputs(const SDL_Event& l_event);

		void ProcessMouseInputs(SDL_Window* l_window);
		void ProcessKeyboardInputs(SDL_Window* l_window);
		
	private:

		static constexpr uint32_t m_totalNumInputKeysToProcess{ (uint32_t)Keys::KEY_MAXIMUM };
		
		std::array<KeyStates, m_totalNumInputKeysToProcess> m_keyStates{};

		std::array<bool, 2> m_mouseStatesPressed;
		std::array<bool, 2> m_mouseStatesUp;

		glm::vec2 m_mousePosRelToWindow{};

		bool m_mouseHidden{ false };
	};

}