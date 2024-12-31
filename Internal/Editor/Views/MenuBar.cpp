//
// Created by wpsimon09 on 31/12/24.
//

#include "MenuBar.hpp"

#include <imgui.h>

namespace VEditor {
    MenuBar::MenuBar(Editor* editor): m_editor(editor)
    {
    }

    void MenuBar::Resize(int newWidth, int newHeight)
    {
    }

    void MenuBar::Render()
    {
        ImGui::BeginMainMenuBar();
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import"))
                {

                }
                if (ImGui::MenuItem("Save"))
                {

                }
                ImGui::EndMenu();
            }

        ImGui::EndMainMenuBar();
        IUserInterfaceElement::Render();
    }

    void MenuBar::Update()
    {
        IUserInterfaceElement::Update();
    }
} // VEditor