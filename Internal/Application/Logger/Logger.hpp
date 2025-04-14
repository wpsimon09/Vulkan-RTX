//
// Created by wpsimon09 on 22/09/24.
//
#pragma once
#ifndef VLOGGER_HPP
#define VLOGGER_HPP

#include <iostream>
#include <chrono>
#include <imgui.h>

namespace VEditor {
class Console;
}

namespace Utils {
enum class ELogType
{
    Success = 0,
    Error,
    Warning,
    Info
};

struct LogEntry
{
    char*    message;
    ELogType type;
};

class Logger
{
  public:
    static void LogSuccess(const std::string& msg);
    static void LogError(const std::string& msg);
    static void LogVKValidationLayerError(const std::string& msg);
    static void LogInfo(const std::string& msg);
    static void LogSuccessClient(const std::string& msg);
    static void LogErrorClient(const std::string& msg);
    static void LogInfoClient(const std::string& msg);
    static void LogInfoVerboseOnlyClient(const std::string& msg);

    static void LogInfoVerboseOnly(const std::string& msg);
    static void LogInfoVerboseRendering(const std::string& msg);

    ~Logger();

  private:
    static void                  AddLogEntry(const std::string& formattedMsg, ELogType type);
    static std::vector<LogEntry> m_logEntries;
    friend VEditor::Console;
};

}  // namespace Utils

#endif  //VLOGGER_HPP
