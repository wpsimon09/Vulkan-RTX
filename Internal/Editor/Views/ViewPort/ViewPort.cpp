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
#include "Vulkan/Global/GlobalVariables.hpp"


VEditor::ViewPort::ViewPort(ViewPortContext& viewPortContext, ApplicationCore::Scene& scene,
                            WindowManager& windowManager): m_viewPortContext(viewPortContext), m_scene(scene), m_windowManager(windowManager)
{
}

void VEditor::ViewPort::Render()
{

    // Render the "Scene view port" windowe
        ImGui::Begin(ICON_FA_CAMERA" Scene view port", &m_isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

            ImGui::SetNextItemAllowOverlap();
            ImGui::BeginMenuBar();

                if (ImGui::MenuItem(ICON_FA_PLUS" Add"))
                {
                    ImGui::OpenPopup("AddPopUp");
                }
                if (ImGui::BeginPopup("AddPopUp")){
                    if (ImGui::BeginMenu(ICON_FA_SHAPES" Meshes"))
                    {
                        if (ImGui::Selectable(ICON_FA_CIRCLE " Sphere")){m_scene.AddSphereToScene();}
                        if (ImGui::Selectable(ICON_FA_CUBE " Cube" )){m_scene.AddCubeToScene();}
                        if (ImGui::Selectable(ICON_FA_SQUARE " Plane")){m_scene.AddPlaneToScene();}

                        ImGui::EndMenu();
                    }

                    if (ImGui::BeginMenu(ICON_FA_MOUNTAIN_SUN" Lights"))
                    {
                        if (ImGui::Selectable(ICON_FA_SUN " Directional")){m_scene.AddDirectionalLight();}
                        if (ImGui::Selectable(ICON_FA_LIGHTBULB " Point" )){m_scene.AddPointLight();}
                        ImGui::EndMenu();
                    }

                    ImGui::EndPopup();
                }
            ImGui::EndMenuBar();

            ImVec2 viewportPanelSize = ImGui::GetWindowSize();
            auto imageWidht = ImVec2{viewportPanelSize.x-20, viewportPanelSize.y-60};
            ImGui::Image((ImTextureID)m_viewPortContext.GetImageDs(), imageWidht);
            if (ImGui::IsWindowHovered())
            {
                // disable gltf input
                if (ImGui::GetIO().MouseClicked[0])
                {
                    auto mousePos = GetMousePositionInViewPort(imageWidht);
                    Utils::Logger::LogInfo("Mouse on frame buffer are X: " + std::to_string(mousePos.x) + ", " + std::to_string(mousePos.y));
                    m_scene.PreformRayCast(mousePos);

                }
                m_windowManager.EnableMovementCapture();
            }
            else
            {
                //enable gltf input
                m_windowManager.DisableMovementCapture();
            }


        ImGui::End();


    IUserInterfaceElement::Render();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
    IUserInterfaceElement::Render();
}

glm::vec2 VEditor::ViewPort::GetMousePositionInViewPort(ImVec2& ImageWidth)
{

    ImVec2 cursorPosInWindow = ImGui::GetMousePos();
    ImVec2 windowPos = ImGui::GetWindowPos();

    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
    ImVec2 imageStartPos = {windowPos.x + contentMin.x, windowPos.y + contentMin.y};

    ImVec2 relativeCursorPos = {cursorPosInWindow.x - imageStartPos.x, cursorPosInWindow.y - imageStartPos.y};

    ;
    float scaleX = GlobalVariables::RenderTargetResolutionWidth / ImageWidth.x;
    float scaleY = GlobalVariables::RenderTargetResolutionHeight / ImageWidth.y;


    if (relativeCursorPos.x >= 0 && relativeCursorPos.x <= contentMax.x - contentMin.x &&
        relativeCursorPos.y >= 0 && relativeCursorPos.y <= contentMax.y - contentMin.y)
    {
        relativeCursorPos.x *= scaleX;
        relativeCursorPos.y *= scaleY;
        return {
            (relativeCursorPos.x/GlobalVariables::RenderTargetResolutionWidth) * 2.f - 1.f,
            1.f - ( (relativeCursorPos.y/GlobalVariables::RenderTargetResolutionHeight) * 2.f)};
    }
    else
    {
        return {-2, -2};
    }
}


