



#include "VRenderer/Logger/ThreadSafeInsertLogOnlyMap.hpp"
#include <fmt/core.h>


namespace VRenderer
{
	ThreadSafeInsertLogOnlyMap::ThreadSafeInsertLogOnlyMap() = default;
	
	void ThreadSafeInsertLogOnlyMap::AddMsg(const uint64_t l_msgID, std::string&& l_msg)
	{
		std::lock_guard lv_lock(m_queueMutex);
		m_loggedMsges.emplace(l_msgID, std::move(l_msg));
	}

	void ThreadSafeInsertLogOnlyMap::ConsoleLogAndFlush()
	{
		std::unique_lock lv_uniqueLock(m_queueMutex);

		std::string lv_finalLogMsg{};
		size_t lv_totalSizeFinalLogMsg{};

		for (const auto& l_loggedMsg : m_loggedMsges) {
			lv_totalSizeFinalLogMsg += l_loggedMsg.second.size() + 1U;
		}

		lv_finalLogMsg.reserve(lv_totalSizeFinalLogMsg);
		
		for (const auto& l_loggedMsg : m_loggedMsges) {
			lv_finalLogMsg.append(l_loggedMsg.second.c_str());
		}

		m_loggedMsges.clear();

		lv_uniqueLock.unlock();

		printf(lv_finalLogMsg.c_str());
		std::fflush(stdout);
	}

}