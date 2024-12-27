//
// Created by wpsimon09 on 21/12/24.
//

#include "Index.hpp"

#include <imgui.h>

VEditor::Index::Index(int width, int height): m_width(width), m_height(height)
{
}

void VEditor::Index::Render()
{
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height));
    ImGui::Begin("Vulkan-RTX Editor");
        ImGui::Text("Hello, from vulkan ty kokot hlupy");

    ImGui::End();
    IUserInterfaceElement::Render();
}

void VEditor::Index::Resize(int newWidth, int newHeight)
{
}
