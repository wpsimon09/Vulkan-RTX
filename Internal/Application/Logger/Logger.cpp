//
// Created by wpsimon09 on 22/09/24.
//

#include "Logger.hpp"

#include "Vulkan/Global/GlobalState.hpp"

void Utils::Logger::LogSuccess(const std::string& msg)
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "✔ LOG::SUCCESS::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - "<< msg << std::endl;
}

void Utils::Logger::LogError(const std::string& msg)
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cerr << "LOG::ERROR::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
}

void Utils::Logger::LogInfo(const std::string& msg)
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "LOG::INFO::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
}

void Utils::Logger::LogSuccessClient(const std::string &msg) {
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "✔ LOG::SUCCESS::CLIENT::[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - "<< msg << std::endl;
}

void Utils::Logger::LogErrorClient(const std::string &msg) {
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cerr << "LOG::ERROR::CLIENT[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
}

void Utils::Logger::LogInfoClient(const std::string &msg) {
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "LOG::INFO::CLIENT[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
}

void Utils::Logger::LogInfoVerboseOnlyClient(const std::string &msg) {
    if(GlobalState::Verbose) {
        auto time = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
        std::tm* localTime = std::localtime(&currentTime);
        std::cout << "VERBOSE::LOG::INFO::CLIENT[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
    }
}

void Utils::Logger::LogInfoVerboseOnly(const std::string &msg) {
  if(GlobalState::Verbose) {
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "VERBOSE::LOG::INFO::VULKAN[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
  }
}

void Utils::Logger::LogInfoVerboseRendering(const std::string &msg) {
    if(GlobalState::VerboseInRendering && GlobalState::Verbose) {
        auto time = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
        std::tm* localTime = std::localtime(&currentTime);
        std::cout << "LOG::INFO::VULKAN::RENDERING[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
    }
}
