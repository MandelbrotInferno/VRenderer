#pragma once



#include "VRenderer/Logger/Category.hpp"
#include "VRenderer/Logger/Level.hpp"

#include <queue>
#include <tracy/Tracy.hpp>
#include <fmt/core.h>
#include <fstream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace VRenderer
{
	class Logger final
	{
	public:

		static Logger& GetInstance();

		void SpawnQueueProcessor();

		template<typename ...Args>
		void FormatMsgAndEnqueue(const Level l_level, const Category l_category, const uint32_t l_lineNumber, const char* l_filePath, std::string&& l_unformattedMsg, Args&&...l_args)
		{
			ZoneScoped;

			if ((uint32_t)m_currentLevel >= (uint32_t)l_level) {
				std::string lv_finalMsg{};
				lv_finalMsg.reserve(2048);

				switch (l_level) {
				case Level::ERROR:
					lv_finalMsg.append("\x1b[31m[Error]");
					break;
				case Level::WARNING:
					lv_finalMsg.append("\x1b[33m[Warning]");
					break;
				case Level::INFO:
					lv_finalMsg.append("\x1b[32m[Info]");
					break;
				}

				switch (l_category) {
				case Category::RENDERING:
					lv_finalMsg.append(" [Rendering]");
					break;
				case Category::PHYSICS:
					lv_finalMsg.append(" [Physics]");
					break;
				case Category::GENERAL:
					lv_finalMsg.append(" [General]");
					break;
				}

				lv_finalMsg.append(" [").append(fmt::format_int(l_lineNumber).c_str()).append("]");
				lv_finalMsg.append(" [").append(l_filePath).append("] : ").append(l_unformattedMsg).append("\n\n");

				lv_finalMsg = fmt::vformat(lv_finalMsg, fmt::make_format_args(l_args...));

				{
					std::lock_guard lv_lock(m_queueMutex);
					m_logMsgs.emplace(std::make_pair(l_category, std::move(lv_finalMsg)));
				}
				m_conditionVar.notify_one();
			}
		}

		void StopQueueProcessor();

		void SetCurrentLevel(const Level l_level);

	private:

		Logger();
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(Logger&&) = delete;

	private:

		std::mutex m_queueMutex{};
		std::condition_variable m_conditionVar{};
		std::queue<std::pair<Category, std::string>> m_logMsgs{};

		std::optional<std::thread> m_queueProcessor{};

		std::fstream m_renderingLogFile{};
		std::fstream m_physicsLogFile{};
		std::fstream m_generalLogFile{};

		Level m_currentLevel{};

		std::thread::id m_mainThreadID{};
		
		bool m_stopQueueProcessor{};
	};
}


#define LOGGING
#ifdef LOGGING

#define START_LOGGING(){\
auto& lv_logger = VRenderer::Logger::GetInstance();\
lv_logger.SpawnQueueProcessor();\
}

#define LOG(l_level, l_category, l_unformattedMsg, ...) {\
auto& lv_logger = VRenderer::Logger::GetInstance();\
lv_logger.FormatMsgAndEnqueue(l_level, l_category, __LINE__, __FILE__, l_unformattedMsg, __VA_ARGS__);\
}


#define END_LOGGING(){\
auto& lv_logger = VRenderer::Logger::GetInstance();\
lv_logger.StopQueueProcessor();\
}

#define SET_LEVEL(l_level){\
auto& lv_logger = VRenderer::Logger::GetInstance();\
lv_logger.SetCurrentLevel(l_level);\
}

#else

#define LOG(l_level, l_category, l_unformattedMsg, ...)
#define START_LOGGING()
#define END_LOGGING()
#define SET_LEVEL(l_level)

#endif