#pragma once



#include "VRenderer/ThreadSafeInsertLogOnlyMap.hpp"



namespace VRenderer
{
	class Logger final
	{
	public:

		Logger& GetInstance();



	private:

		Logger();
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(Logger&&) = delete;


	private:

		ThreadSafeInsertLogOnlyMap m_map{};

	};
}