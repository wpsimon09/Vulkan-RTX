#include "Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include <cstring>

std::vector<Utils::LogEntry> Utils::Logger::m_logEntries;

void Utils::Logger::AddLogEntry(const std::string& formattedMsg, ELogType type) {
    char* messageCopy = new char[formattedMsg.length() + 1];
    std::strcpy(messageCopy, formattedMsg.c_str());
    m_logEntries.push_back({ messageCopy, type });
}

void Utils::Logger::LogSuccess(const std::string& msg)
{
    if (!GlobalState::LoggingEnabled) {
        return;
    }

    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "✔ LOG::SUCCESS::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Success);

    std::cout << "================================================================================================================" << std::endl;
    std::cout << oss.str() << std::endl;
    std::cout << "================================================================================================================" << std::endl;
}

void Utils::Logger::LogError(const std::string& msg)
{
    if (!GlobalState::LoggingEnabled) {
        return;
    }

    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "LOG::ERROR::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Error);

    std::cerr << "=================================================================================================================================================================================" << std::endl;
    std::cerr << oss.str() << std::endl;
    std::cerr << "=================================================================================================================================================================================" << std::endl;
}

void Utils::Logger::LogInfo(const std::string& msg)
{
    if (!GlobalState::LoggingEnabled) {
        return;
    }

    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "LOG::INFO::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Warning);

    std::cout << oss.str() << std::endl;
}

void Utils::Logger::LogSuccessClient(const std::string &msg) {
    if (!GlobalState::LoggingEnabled) {
        return;
    }

    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "✔ LOG::SUCCESS::CLIENT::[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Success);

    std::cout << oss.str() << std::endl;
}

void Utils::Logger::LogErrorClient(const std::string &msg) {
    if (!GlobalState::LoggingEnabled) {
        return;
    }

    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "LOG::ERROR::CLIENT[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Error);

    std::cerr << oss.str() << std::endl;
}

void Utils::Logger::LogInfoClient(const std::string &msg) {
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "LOG::INFO::CLIENT[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Warning);

    std::cout << oss.str() << std::endl;
}

void Utils::Logger::LogInfoVerboseOnlyClient(const std::string &msg) {
    if (GlobalState::Verbose && GlobalState::LoggingEnabled) {
        auto time = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
        std::tm* localTime = std::localtime(&currentTime);
        std::ostringstream oss;
        oss << "VERBOSE::LOG::INFO::CLIENT[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
        AddLogEntry(oss.str(), ELogType::Warning);

        std::cout << oss.str() << std::endl;
    }
}

void Utils::Logger::LogInfoVerboseOnly(const std::string &msg) {
    if (GlobalState::Verbose && GlobalState::LoggingEnabled) {
        auto time = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
        std::tm* localTime = std::localtime(&currentTime);
        std::ostringstream oss;
        oss << "VERBOSE::LOG::INFO::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
        AddLogEntry(oss.str(), ELogType::Warning);

        std::cout << oss.str() << std::endl;
    }
}

void Utils::Logger::LogInfoVerboseRendering(const std::string& msg)
{
    if (!GlobalState::VerboseInRendering)
        return;
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << "VERBOSE::LOG::INFO::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg;
    AddLogEntry(oss.str(), ELogType::Warning);

    std::cout << oss.str() << std::endl;
}
