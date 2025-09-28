//
// Created by wpsimon09 on 21/12/24.
//

#include "ViewPort.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <IconFontCppHeaders/IconsFontAwesome6.h>
#include <ImGuizmo/ImGuizmo.h>

#include "Application/Enums/ClientEnums.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Editor/UIContext/ViewPortContext.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"


VEditor::ViewPort::ViewPort(std::unordered_map<ViewPortType, ViewPortContext>& viewPorts,
                            ViewPortContext&                                   rasterViewportContext,
                            ViewPortContext&                                   rayTracedViewPortContext,
                            bool&                                              isRayTracing,
                            ApplicationCore::Scene&                            scene,
                            WindowManager&                                     windowManager,
                            ApplicationCore::ApplicationState&                 applicationState)
    : m_rasterViewPortContext(rasterViewportContext)
    , m_rayTracedViewPortContext(rayTracedViewPortContext)
    , m_isRayTracing(isRayTracing)
    , m_scene(scene)
    , m_windowManager(windowManager)
    , m_viewPorts(viewPorts)
    , m_applicationSate(applicationState)

{
    m_previousHeight = rasterViewportContext.height;
    m_previousWidth  = rasterViewportContext.width;
}

void VEditor::ViewPort::Render()
{

    // Render the "Scene view port" windowe
    ImGui::Begin(ICON_FA_CAMERA " Scene view port", &m_isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);


    ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);

    ImGui::SetNextItemAllowOverlap();
    ImGui::BeginMenuBar();

    if(ImGui::MenuItem(ICON_FA_PLUS " Add"))
    {
        ImGui::OpenPopup("AddPopUp");
    }

    if(ImGui::MenuItem(ICON_FA_CAMERA " View port"))
    {
        ImGui::OpenPopup("ViewPortPopup");
    }

    RenderViewPortSelection();

    if(ImGui::BeginPopup("AddPopUp"))
    {
        if(ImGui::BeginMenu(ICON_FA_SHAPES " Meshes"))
        {
            if(ImGui::Selectable(ICON_FA_CIRCLE " Sphere"))
            {
                m_scene.AddSphereToScene();
            }
            if(ImGui::Selectable(ICON_FA_CUBE " Cube"))
            {
                m_scene.AddCubeToScene();
            }
            if(ImGui::Selectable(ICON_FA_SQUARE " Plane"))
            {
                m_scene.AddPlaneToScene();
            }

            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu(ICON_FA_WAND_MAGIC_SPARKLES " Effects"))
        {
            if(ImGui::Selectable(ICON_FA_SMOG " Fog volume"))
            {
                m_scene.AddFogVolume();
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu(ICON_FA_SUN " Lights"))
        {
            if (ImGui::Selectable( ICON_FA_CLOUD_SUN "Atmosphere "))
                {
                m_scene.AddAtmosphere();
            }
            if(ImGui::Selectable(ICON_FA_SUN " Directional"))
            {
                m_scene.AddDirectionalLight();
            }
            if(ImGui::Selectable(ICON_FA_LIGHTBULB " Point"))
            {
                m_scene.AddPointLight();
            }
            if(ImGui::Selectable(ICON_FA_SQUARE " Area"))
            {
                m_scene.AddAreaLight();
            }
            if(ImGui::Selectable(ICON_FA_MOUNTAIN_SUN " Sky-Box"))
            {
                m_scene.AddSkyBox();
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::EndMenuBar();

    ImVec2 viewportPanelSize = ImGui::GetWindowSize();
    auto   imageSize         = ImVec2{viewportPanelSize.x - 20, viewportPanelSize.y - 60};
    auto   imageOrigin       = ImGui::GetCursorScreenPos();
    m_gizmoRectOriginX       = imageOrigin.x;
    m_gizmoRectOriginY       = imageOrigin.y;


    if(m_isRayTracing)
    {
        ImGui::Image((ImTextureID)m_viewPorts[ViewPortType::eMainRayTracer].GetImageDs(), imageSize);
    }
    else
    {
        ImGui::Image((ImTextureID)m_viewPorts[m_selectedViewPort].GetImageDs(), imageSize);
    }
    ImGuizmo::SetRect(m_gizmoRectOriginX, m_gizmoRectOriginY, imageSize.x, imageSize.y);

    RenderGizmoActions(imageOrigin, imageSize);

    if(ImGui::IsWindowHovered())
    {
        if(!ImGuizmo::IsOver() && !ImGuizmo::IsViewManipulateHovered())
        {
            // disable gltf input
            if(ImGui::GetIO().MouseClicked[0])
            {
                auto mousePos = GetMousePositionInViewPort(imageSize);
                Utils::Logger::LogInfo("Mouse on frame buffer are X: " + std::to_string(mousePos.x) + ", "
                                       + std::to_string(mousePos.y));
                m_scene.PreformRayCast(mousePos);
            }
        }
        m_windowManager.EnableMovementCapture();
    }
    else
    {
        //enable gltf input

        m_windowManager.DisableMovementCapture();
    }


    ImGui::End();


    if(m_previousHeight != imageSize.y || m_previousWidth != imageSize.x)
    {
        Resize(imageSize.x, imageSize.y);
    }


    IUserInterfaceElement::Render();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
    m_previousHeight = newHeight;
    m_previousWidth  = newWidth;

    m_rasterViewPortContext.camera->ProcessResize(newWidth, newHeight);
    m_rasterViewPortContext.hasResized = true;


    ImGuizmo::SetRect(m_gizmoRectOriginX, m_gizmoRectOriginY, newWidth, newHeight);
    IUserInterfaceElement::Render();
}
void VEditor::ViewPort::SetViewPortContext(ViewPortContext& viewPortContext)
{
    m_rasterViewPortContext = viewPortContext;
}

void VEditor::ViewPort::RenderGizmoActions(ImVec2& imageOrigin, ImVec2& imageSize)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));         // Dark gray with 50% transparency
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.8f));  // Slightly brighter when hovered
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));   // Fully visible when clicked

    ImGui::SetCursorScreenPos({imageOrigin.x, imageOrigin.y});

    if(ImGui::Button(ICON_FA_UP_DOWN_LEFT_RIGHT))
    {
        ImGuizmo::currentOperation = ImGuizmo::TRANSLATE;
    }
    ImGui::SetItemTooltip("Translate");

    if(ImGui::Button(ICON_FA_ROTATE))
    {
        ImGuizmo::currentOperation = ImGuizmo::ROTATE;
    }
    ImGui::SetItemTooltip("Rotate");

    if(ImGui::Button(ICON_FA_EXPAND))
    {
        ImGuizmo::currentOperation = ImGuizmo::SCALE;
    }
    ImGui::SetItemTooltip("Scale");


    if(ImGuizmo::CURRENT_MODE == ImGuizmo::WORLD)
    {
        if(ImGui::Button(ICON_FA_EARTH_EUROPE))
        {
            ImGuizmo::CURRENT_MODE = ImGuizmo::LOCAL;
        }
        ImGui::SetItemTooltip("World space");
    }
    if(ImGuizmo::CURRENT_MODE == ImGuizmo::LOCAL)
    {
        if(ImGui::Button(ICON_FA_ARROW_UP_RIGHT_FROM_SQUARE))
        {
            ImGuizmo::CURRENT_MODE = ImGuizmo::WORLD;
        }
        ImGui::SetItemTooltip("Local space ");
    }
    ImGui::PopStyleColor(3);

    auto view = m_scene.GetCamera().GetViewMatrix();
    ImGuizmo::ViewManipulate(glm::value_ptr(view), 10, ImVec2(imageSize.x - 100, imageSize.y), ImVec2(100.f, 100.f), 32);
    auto newCamPos = glm::vec3(glm::inverse(view)[3]);
    m_scene.GetCamera().SetPosition(newCamPos);
}
void VEditor::ViewPort::RenderViewPortSelection()
{
    if(ImGui::BeginPopup("ViewPortPopup"))
    {
        int i = 0;
        ImGui::SeparatorText("Buffers");
        for(auto viewPort : m_viewPortOptions)
        {
            if(ImGui::Selectable(viewPort, static_cast<ViewPortType>(i) == m_selectedViewPort))
            {
                m_selectedViewPort = static_cast<ViewPortType>(i);
            }

            i++;
        }

        ImGui::SeparatorText("Data");

        int j = 0;

        for(auto debuData : m_debugViews)
        {
            if(ImGui::Selectable(debuData, static_cast<EDebugRendering>(j) == m_applicationSate.m_rendererOutput))
            {
                m_applicationSate.m_rendererOutput = static_cast<EDebugRendering>(j);
            }
            j++;
        }


        ImGui::SeparatorText("Ray tracing");

        j = 0;
        for(auto debugData : m_debugViewRayTracing)
        {
            if(ImGui::Selectable(debugData, static_cast<EDebugRenderingRTX>(j) == m_applicationSate.m_rtxRenderOutput))
            {
                m_applicationSate.m_rtxRenderOutput = static_cast<EDebugRenderingRTX>(j);
            }
            j++;
        }


        ImGui::EndPopup();
    }
}

glm::vec2 VEditor::ViewPort::GetMousePositionInViewPort(ImVec2& ImageWidth)
{

    ImVec2 cursorPosInWindow = ImGui::GetMousePos();
    ImVec2 windowPos         = ImGui::GetWindowPos();

    ImVec2 contentMin    = ImGui::GetWindowContentRegionMin();
    ImVec2 contentMax    = ImGui::GetWindowContentRegionMax();
    ImVec2 imageStartPos = {windowPos.x + contentMin.x, windowPos.y + contentMin.y};

    ImVec2 relativeCursorPos = {cursorPosInWindow.x - imageStartPos.x, cursorPosInWindow.y - imageStartPos.y};

    float scaleX = GlobalVariables::RenderTargetResolutionWidth / ImageWidth.x;
    float scaleY = GlobalVariables::RenderTargetResolutionHeight / ImageWidth.y;


    if(relativeCursorPos.x >= 0 && relativeCursorPos.x <= contentMax.x - contentMin.x && relativeCursorPos.y >= 0
       && relativeCursorPos.y <= contentMax.y - contentMin.y)
    {
        relativeCursorPos.x *= scaleX;
        relativeCursorPos.y *= scaleY;
        return {(relativeCursorPos.x / GlobalVariables::RenderTargetResolutionWidth) * 2.f - 1.f,
                1.f - ((relativeCursorPos.y / GlobalVariables::RenderTargetResolutionHeight) * 2.f)};
    }
    else
    {
        return {-2, -2};
    }
}
