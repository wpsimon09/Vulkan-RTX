//
// Created by wpsimon09 on 18/01/25.
//

#include "Console.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

namespace VEditor {
    Console::Console()
    {
    }

    void Console::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS" Console");

        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void Console::Resize(int newWidth, int newHeight)
    {
    }
} // VEditor