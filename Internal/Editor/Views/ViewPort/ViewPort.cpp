//
// Created by wpsimon09 on 21/12/24.
//

#include "ViewPort.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include "Application/Rendering/Scene/Scene.hpp"
#include "Editor/UIContext/ViewPortContext.hpp"

VEditor::ViewPort::ViewPort(ViewPortContext& viewPortContext, const ApplicationCore::Scene& scene): m_viewPortContext(viewPortContext),m_scene(scene), IUserInterfaceElement{}
{
}

void VEditor::ViewPort::Render()
{

    // Render the "Scene view port" window
    ImGui::Begin(ICON_FA_CAMERA" Scene view port", &m_isOpen, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button(ICON_FA_SHAPES" Add"))
        {
            ImGui::OpenPopup("Meshes");
        }
        if (ImGui::BeginPopup("Meshes")){
            if (ImGui::Selectable(ICON_FA_CIRCLE " Sphere")){m_scene.AddSphereToScene();}
            if (ImGui::Selectable(ICON_FA_CUBE " Cube" )){m_scene.AddCubeToScene();}
            if (ImGui::Selectable(ICON_FA_SQUARE " Plane")){m_scene.AddPlaneToScene();}

            ImGui::EndPopup();
        }

    ImVec2 viewportPanelSize = ImGui::GetWindowSize();
        ImGui::Image((ImTextureID)m_viewPortContext.GetImageDs(), ImVec2{viewportPanelSize.x, viewportPanelSize.y});


        ImGui::End();


    IUserInterfaceElement::Render();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
    IUserInterfaceElement::Render();
}


