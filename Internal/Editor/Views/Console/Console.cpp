//
// Created by wpsimon09 on 18/01/25.
//

#include "Console.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"

namespace VEditor {
    Console::Console() = default;

    void Console::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_OPEN" Console");

        if (Utils::Logger::m_logEntries.size() > GlobalState::LogLimit)
        {
            Utils::Logger::m_logEntries.clear();
        }

        if (Utils::Logger::m_logEntries.size() > m_previousNumberOfLogs )
            m_scrollToBottom = true;

        if (ImGui::Button("Clear"))
        {
            for (auto &logEntry : Utils::Logger::m_logEntries){
                delete logEntry.message;
            }
            Utils::Logger::m_logEntries.clear();
        }
        ImGui::BeginChild("Console");
        for (auto &logEntry : Utils::Logger::m_logEntries)
        {
            if (logEntry.message[0] != '\0')
            {

                const char* icon;
                ImVec4 color;
                if (logEntry.type == Utils::ELogType::Success)
                {
                    icon = ICON_FA_CHECK;
                    color = ImColor(60, 205, 50);
                }
                else if (logEntry.type == Utils::ELogType::Error)
                {
                    icon = ICON_FA_BUG;
                    color = ImColor(255, 0, 0);
                }
                else if (logEntry.type == Utils::ELogType::Info)
                {
                    icon = ICON_FA_INFO;
                    color = ImColor(200, 200, 200);
                }

                std::string logMessage = std::string(icon) + " " + logEntry.message;
                ImGui::TextColored(color, "%s", logMessage.c_str());

                if (m_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                    m_scrollToBottom = false;
                }

            }
            m_previousNumberOfLogs = Utils::Logger::m_logEntries.size();

        }
        ImGui::EndChild();
        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void Console::Resize(int newWidth, int newHeight)
    {
    }
} // VEditor