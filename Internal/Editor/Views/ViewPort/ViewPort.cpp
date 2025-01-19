//
// Created by wpsimon09 on 21/12/24.
//

#include "ViewPort.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Editor/UIContext/ViewPortContext.hpp"


VEditor::ViewPort::ViewPort(ViewPortContext& viewPortContext, const ApplicationCore::Scene& scene,
    WindowManager& windowManager): m_viewPortContext(viewPortContext), m_scene(scene), m_windowManager(windowManager)
{
}

void VEditor::ViewPort::Render()
{

    // Render the "Scene view port" windowe
        ImGui::Begin(ICON_FA_CAMERA" Scene view port", &m_isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

            ImGui::SetNextItemAllowOverlap();
            ImGui::BeginMenuBar();

                if (ImGui::MenuItem(ICON_FA_SHAPES" Add"))
                {
                    ImGui::OpenPopup("Meshes");
                }
                if (ImGui::BeginPopup("Meshes")){
                    if (ImGui::Selectable(ICON_FA_CIRCLE " Sphere")){m_scene.AddSphereToScene();}
                    if (ImGui::Selectable(ICON_FA_CUBE " Cube" )){m_scene.AddCubeToScene();}
                    if (ImGui::Selectable(ICON_FA_SQUARE " Plane")){m_scene.AddPlaneToScene();}

                    ImGui::EndPopup();
                }
            ImGui::EndMenuBar();

            if (ImGui::IsWindowHovered())
            {
                // disable gltf input
                m_windowManager.EnableMovementCapture();


                ImVec2 cursorPosInWindow = ImGui::GetMousePos();
                ImVec2 windowPos = ImGui::GetWindowPos();

                ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
                ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
                ImVec2 imageStartPos = {windowPos.x + contentMin.x, windowPos.y + contentMin.y};

                ImVec2 relativeCursorPos = {cursorPosInWindow.x - imageStartPos.x, cursorPosInWindow.y - imageStartPos.y};

                if (relativeCursorPos.x >= 0 && relativeCursorPos.x <= contentMax.x - contentMin.x &&
        relativeCursorPos.y >= 0 && relativeCursorPos.y <= contentMax.y - contentMin.y)
                {
                    auto s = "Mouse on image:" + std::to_string(relativeCursorPos.x) + "," + std::to_string(relativeCursorPos.y);
                    ImGui::Text(s.c_str());
                }
                else
                {
                    ImGui::Text("Mouse outside image bounds");
                }


                }
            else
            {
                //enable gltf input
                m_windowManager.DisableMovementCapture();
            }
            ImVec2 viewportPanelSize = ImGui::GetWindowSize();
            ImGui::Image((ImTextureID)m_viewPortContext.GetImageDs(), ImVec2{viewportPanelSize.x-20, viewportPanelSize.y-60});


        ImGui::End();


    IUserInterfaceElement::Render();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
    IUserInterfaceElement::Render();
}


