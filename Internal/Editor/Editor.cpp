//
// Created by wpsimon09 on 27/12/24.
//

#include "Editor.hpp"

#include "Views/Index.hpp"
#include "UIContext/UIContext.hpp"
#include "Views/SceneView/SceneView.hpp"
#include "Views/ViewPort/ViewPort.hpp"

namespace VEditor
{
    Editor::Editor(UIContext& uiContext): m_uiContext(uiContext)
    {
        Utils::Logger::LogInfo("Initialization of visual editor");
        auto start = std::chrono::high_resolution_clock::now();

        auto index = std::make_unique<VEditor::Index>(uiContext.m_windowManager.GetWindowWidth(),
                                                      uiContext.m_windowManager.GetWindowHeight());

        auto viewPort = std::make_unique<ViewPort>(uiContext.m_viewports[ViewPortType::eMain]);
        index->m_uiChildren.emplace_back(std::move(viewPort));

        auto sceneView = std::make_unique<SceneView>(uiContext.m_scene);
        index->m_uiChildren.emplace_back(std::move(sceneView));

        m_uiElements.emplace_back(std::move(index));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        Utils::Logger::LogInfo("Visual editor initialized in: " + std::to_string(duration.count()) + "ms !");
    }

    void Editor::Render()
    {
        m_uiContext.BeginRender();
        RenderPrefomanceOverlay();
        for (auto& uiElement : m_uiElements)
        {
            uiElement->Render();
        }
        m_uiContext.EndRender();
    }

    void Editor::Update()
    {
        if (m_uiContext.m_windowManager.GetHasResizedStatus())
        {
            for (auto& uiElement : m_uiElements)
            {
                uiElement->Resize(m_uiContext.m_windowManager.GetWindowWidth(),
                                  m_uiContext.m_windowManager.GetWindowWidth());
            }
        }
    }

    void Editor::RenderPrefomanceOverlay()
    {
        static int location = 0;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (location >= 0)
        {
            const float PAD = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos;
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        else if (location == -2)
        {
            // Center window
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        bool isOpne = true;
        if (ImGui::Begin("Stat",&isOpne , window_flags))
        {
            ImGui::Text("(right-click to change position)");
            ImGui::Separator();
            ImGui::Text("Prefomance MS/FPS: (%.1f,%.1f)", 1000.0f / io.Framerate, io.Framerate);
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
                if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
                if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
                if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
                if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
                if (isOpne&& ImGui::MenuItem("Close")) isOpne = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
} // VEditor
