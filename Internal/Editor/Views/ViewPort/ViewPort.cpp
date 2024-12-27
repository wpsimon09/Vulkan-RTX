//
// Created by wpsimon09 on 21/12/24.
//

#include "ViewPort.hpp"

#include <imgui.h>

VEditor::ViewPort::ViewPort(ViewPortContext& viewPortContext): m_viewPortContext(viewPortContext), IUserInterfaceElement{}
{
}

void VEditor::ViewPort::Render()
{
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::Begin("Scene view port");
    ImGui::Text("Jooj zdochni geňo");

    ImGui::End();

    IUserInterfaceElement::Render();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
    IUserInterfaceElement::Render();
}


