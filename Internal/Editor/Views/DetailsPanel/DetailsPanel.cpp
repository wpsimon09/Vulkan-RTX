//
// Created by wpsimon09 on 29/12/24.
//

#include "DetailsPanel.hpp"
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include <imgui.h>

namespace VEditor {
    DetailsPanel::DetailsPanel()
    {

    }

    void DetailsPanel::Render()
    {
        ImGui::Begin(ICON_FA_PEN_FANCY " Details");
            if (!m_selectedSceneNode)
            {
                ImGui::TextColored(ImVec4(0.9, 0.2,0.2,1.0), "No scene node selected");
            }

        ImGui::End();
        IUserInterfaceElement::Render();
    }

    void DetailsPanel::Resize(int newWidth, int newHeight)
    {

    }
} // VEditor