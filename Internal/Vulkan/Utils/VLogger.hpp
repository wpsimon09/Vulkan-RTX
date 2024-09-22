//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VLOGGER_HPP
#define VLOGGER_HPP

#include <iostream>
#include <chrono>

namespace Utils
{
    class VLogger
    {
    public:
        static void LogSuccess(const std::string &msg);
        static void LogError(const std::string &msg);
        static void LogInfo(const std::string &msg);
    private:

    };
}

#endif //VLOGGER_HPP
