//
// Created by wpsimon09 on 22/09/24.
//

#include "VLogger.hpp"

void Utils::VLogger::LogSuccess(const std::string& msg)
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "✔ LOG::SUCCESS::[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - "<< msg << std::endl;
}

void Utils::VLogger::LogError(const std::string& msg)
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cerr << "LOG::ERROR::[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
}

void Utils::VLogger::LogInfo(const std::string& msg)
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    std::cout << "LOG::INFO::[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - " << msg << std::endl;
}
