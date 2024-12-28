//
// Created by wpsimon09 on 21/12/24.
//

#include "ViewPort.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include "Editor/UIContext/ViewPortContext.hpp"

VEditor::ViewPort::ViewPort(ViewPortContext& viewPortContext): m_viewPortContext(viewPortContext), IUserInterfaceElement{}
{
}

void VEditor::ViewPort::Render()
{

    // Render the "Scene view port" window
    ImGui::Begin("Scene view port");
        ImVec2 viewportPanelSize = ImGui::GetWindowSize();

        ImGui::Image((ImTextureID)m_viewPortContext.GetImageDs(), ImVec2{viewportPanelSize.x, viewportPanelSize.y});
    ImGui::End();

    IUserInterfaceElement::Render();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
    IUserInterfaceElement::Render();
}


