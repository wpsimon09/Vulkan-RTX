//
// Created by wpsimon09 on 20/09/24.
//
module;
#include <iostream>
#include <chrono>
#include <ctime>

export module Logger;

namespace Logger
{
    auto time = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(time);
    std::tm* localTime = std::localtime(&currentTime);
    export void LogSuccess(const std::string &msg)
    {
        std::cout << "✔ [" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] - LOG::SUCCESS::  "<< msg << std::endl;
    }
    export void LogError(const std::string &msg)
    {
        std::cerr << "LOG::ERROR::[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] " << msg << std::endl;
    }
}

