#pragma once


#include <mutex>
#include <map>
#include <string>


namespace VRenderer
{
	class ThreadSafeInsertLogOnlyMap final
	{
	public:

		ThreadSafeInsertLogOnlyMap();

		ThreadSafeInsertLogOnlyMap(const ThreadSafeInsertLogOnlyMap&) = delete;
		ThreadSafeInsertLogOnlyMap& operator=(const ThreadSafeInsertLogOnlyMap&) = delete;

		void AddMsg(const uint64_t l_msgID, std::string&& l_msg);

		void ConsoleLogAndFlush();

	private:

		mutable std::mutex m_queueMutex{};
		std::map<uint64_t, std::string> m_loggedMsges;
	};
}