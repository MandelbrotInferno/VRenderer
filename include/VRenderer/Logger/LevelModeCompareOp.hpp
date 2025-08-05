#pragma once




#include <cinttypes>



namespace VRenderer
{
	enum class LevelModeCompareOp : uint32_t
	{
		EQUAL = 0U,
		EQUAL_LESS,
		EQUAL_MORE
	};
}