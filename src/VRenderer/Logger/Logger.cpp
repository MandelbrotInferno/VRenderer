


#include "VRenderer/Logger/Logger.hpp"




namespace VRenderer
{
	Logger::Logger()
	{
		m_mainThreadID = std::this_thread::get_id();

		m_renderingLogFile.open("C:/Users/farhan/source/repos/VRenderer/Logs/Rendering.txt", std::ios::trunc | std::ios::out);
		if (false == m_renderingLogFile.is_open()) {
			m_renderingLogFile.exceptions(std::ios::failbit);
		}
		m_physicsLogFile.open("C:/Users/farhan/source/repos/VRenderer/Logs/Physics.txt", std::ios::trunc | std::ios::out);
		if (false == m_physicsLogFile.is_open()) {
			m_physicsLogFile.exceptions(std::ios::failbit);
		}
		m_generalLogFile.open("C:/Users/farhan/source/repos/VRenderer/Logs/General.txt", std::ios::trunc | std::ios::out);
		if (false == m_generalLogFile.is_open()) {
			m_generalLogFile.exceptions(std::ios::failbit);
		}
	}

	Logger& Logger::GetInstance()
	{
		static Logger lv_logger;
		return lv_logger;
	}


	void Logger::StopQueueProcessor()
	{
		m_stopQueueProcessor = true;
		m_conditionVar.notify_one();
		if (true == m_queueProcessor.has_value()) {
			m_queueProcessor.value().join();
		}

		std::lock_guard lv_lock(m_queueMutex);
		while (false == m_logMsgs.empty()) {
			
			const auto lv_msgToProcess = std::move(m_logMsgs.front());
			m_logMsgs.pop();

			if (Category::GENERAL == lv_msgToProcess.first) {
				m_generalLogFile << lv_msgToProcess.second;
				m_generalLogFile.flush();
			}
			else if (Category::RENDERING == lv_msgToProcess.first) {
				m_renderingLogFile << lv_msgToProcess.second;
				m_renderingLogFile.flush();
			}
			else if (Category::PHYSICS == lv_msgToProcess.first) {
				m_physicsLogFile << lv_msgToProcess.second;
				m_physicsLogFile.flush();
			}

			printf(lv_msgToProcess.second.c_str());
			std::fflush(stdout);
		}
	}


	void Logger::SpawnQueueProcessor()
	{
		if (false == m_queueProcessor.has_value()) {

			auto lv_thread = [this]()->void
				{
					while (false == m_stopQueueProcessor) {
						ZoneScoped;

						std::unique_lock lv_lock(m_queueMutex);
						m_conditionVar.wait(lv_lock, [this]() {return (!m_logMsgs.empty()) || (true == m_stopQueueProcessor); });

						if (false == m_logMsgs.empty()) {
							const auto lv_msgToProcess = std::move(m_logMsgs.front());
							m_logMsgs.pop();
							lv_lock.unlock();

							if (Category::GENERAL == lv_msgToProcess.first) {
								m_generalLogFile << lv_msgToProcess.second;
								m_generalLogFile.flush();
							}
							else if (Category::RENDERING == lv_msgToProcess.first) {
								m_renderingLogFile << lv_msgToProcess.second;
								m_renderingLogFile.flush();
							}
							else if (Category::PHYSICS == lv_msgToProcess.first) {
								m_physicsLogFile << lv_msgToProcess.second;
								m_physicsLogFile.flush();
							}

							printf(lv_msgToProcess.second.c_str());
							std::fflush(stdout);
						}
					}
				};

			m_queueProcessor.emplace(lv_thread);

		}
	}

	void Logger::SetCurrentLevel(const Level l_level)
	{
		if (m_mainThreadID == std::this_thread::get_id()) {
			m_currentLevel = l_level;
		}
	}
}