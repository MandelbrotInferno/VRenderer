#pragma once


#include <cinttypes>

namespace GhazaEngine
{
	//All times in milliseconds
	struct Time final
	{
	public:

		uint64_t m_currentTime{};
		uint64_t m_deltaTime{};
	};

}