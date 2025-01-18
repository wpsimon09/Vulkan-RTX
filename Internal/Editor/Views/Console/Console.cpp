//
// Created by wpsimon09 on 18/01/25.
//

#include "Console.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/Logger/Logger.hpp"

namespace VEditor {
    Console::Console()
    {
    }

    void Console::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS" Console");

        for (auto &logEntry : Utils::Logger::m_logEntries)
        {
            if (logEntry.message[0] != '\0')
            {

                const char* icon;
                ImVec4 color;
                if (logEntry.type == Utils::ELogType::Success)
                {
                    icon = ICON_FA_CHECK;
                    color = ImColor(0, 155, 0);
                }
                else if (logEntry.type == Utils::ELogType::Error)
                {
                    icon = ICON_FA_CROSS;
                    color = ImColor(255, 0, 0);
                }

                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                std::string logMessage = std::string(icon) + " " + logEntry.message;
                ImGui::TextColored(color, "%s", logMessage.c_str());

            }

        }
        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void Console::Resize(int newWidth, int newHeight)
    {
    }
} // VEditor