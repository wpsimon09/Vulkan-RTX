//
// Created by wpsimon09 on 27/12/24.
//

#include "Editor.hpp"

#include <IconsFontAwesome6.h>

#include "Application/Client.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Views/Index.hpp"
#include "UIContext/UIContext.hpp"
#include "Views/MenuBar.hpp"
#include "Views/FileExplorer/FileExplorer.hpp"
#include "Views/RenderingOptions/RenderingOptions.hpp"
#include "Views/SceneView/SceneView.hpp"
#include "Views/ViewPort/ViewPort.hpp"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"

namespace VEditor
{
    Editor::Editor(UIContext& uiContext): m_uiContext(uiContext)
    {
        Utils::Logger::LogInfo("Initialization of visual editor");
        auto start = std::chrono::high_resolution_clock::now();

        auto index = std::make_unique<VEditor::Index>(uiContext.m_windowManager.GetWindowWidth(),
                                                      uiContext.m_windowManager.GetWindowHeight());

        auto viewPort = std::make_unique<ViewPort>(uiContext.m_viewports[ViewPortType::eMain], m_uiContext.m_client.GetScene());
        index->m_uiChildren.emplace_back(std::move(viewPort));

        auto sceneGraph = std::make_unique<SceneView>(uiContext.GetScene());
        index->m_uiChildren.emplace_back(std::move(sceneGraph));

        auto renderingSystem = std::make_unique<RenderingOptions>(uiContext.m_renderingSystem);
        index->m_uiChildren.emplace_back(std::move(renderingSystem));

        auto menuBar = std::make_unique<MenuBar>(this);

        m_uiElements.emplace_back(std::move(index));
        m_uiElements.emplace_back(std::move(menuBar));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);


        Utils::Logger::LogInfo("Visual editor initialized in: " + std::to_string(duration.count()) + "ms !");
    }

    void Editor::Render()
    {
        m_uiContext.BeginRender();
        RenderPreformanceOverlay();
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
                uiElement->Update();
            }
        }
    }

    void Editor::RenderPreformanceOverlay() const
    {
        static int location = 2;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize |
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
        ImGui::SetNextWindowBgAlpha(0.75f); // Transparent background
        bool isOpen = true;
        if (ImGui::Begin("Stat",&isOpen , window_flags))
        {
            ImGui::Text("Selected path %s", m_filePath.string().empty() ? "<empty>" : m_filePath.string().c_str());

            ImGui::Text("Prefomance MS/FPS: (%.1f,%.1f)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::SeparatorText("GPU");
            ImGui::Text("GPU:");
            ImGui::SameLine();
            ImGui::Text(GlobalVariables::GlobalStructs::GpuProperties.deviceName);
            ImGui::Button(ICON_FA_INFO);
            if (ImGui::BeginItemTooltip())
            {
                ImGui::Text("Memory usage of the GPU ");
                float available;
                float used;


                for (uint32_t i = 0; i < GlobalVariables::GlobalStructs::GpuMemoryProperties.memoryHeapCount; ++i) {
                    if (GlobalVariables::GlobalStructs::GpuMemoryProperties.memoryHeaps[i].flags && VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                        available += GlobalVariables::GlobalStructs::GpuMemoryProperties.memoryHeaps[i].size;
                        used += m_vmaStats->memoryHeap[i].statistics.allocationBytes;
                    }
                }
                used *= (1024.0f * 1024.0f * 1024.0f);
                available *= (1024.0f * 1024.0f * 1024.0f);
                ImGui::Text("VRAM");
                ImGui::ProgressBar(used/available);

                ImGui::SeparatorText("Draw stat");
                {
                    auto &sceneStats =  m_uiContext.GetScene().GetSceneStatistics();
                    //ImGui::Text("Draw calls %i",sceneStats.drawCalls);
                    ImGui::Text("Mesh count %i",sceneStats.numberOfMeshes);
                }

                ImGui::EndTooltip();
            }

            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom", NULL, location == -1)) location = -1;
                if (ImGui::MenuItem("Center", NULL, location == -2)) location = -2;
                if (ImGui::MenuItem("Top-left", NULL, location == 0)) location = 0;
                if (ImGui::MenuItem("Top-right", NULL, location == 1)) location = 1;
                if (ImGui::MenuItem("Bottom-left", NULL, location == 2)) location = 2;
                if (ImGui::MenuItem("Bottom-right", NULL, location == 3)) location = 3;
                if (isOpen&& ImGui::MenuItem("Close")) isOpen = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
} // VEditor
