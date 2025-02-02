//
// Created by wpsimon09 on 30/12/24.
//

#include "RenderingOptions.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"

namespace VEditor {
    RenderingOptions::RenderingOptions(Renderer::RenderingSystem* renderingSystem)
    {
        m_renderingSystem = renderingSystem;
    }


    void RenderingOptions::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS " Rendering options", &m_isOpen);

            ImGui::Checkbox("Fake ray-tracer ", &m_renderingSystem->m_isRayTracing);
            ImGui::Checkbox("Editor billboards ", &m_renderingSystem->m_allowEditorBillboards);

            if (ImGui::TreeNode(ICON_FA_DRAW_POLYGON " SceneRenderer"))
            {
                ImGui::Checkbox("Wire frame mode", &m_renderingSystem->m_sceneRenderer->m_WireFrame);

                ImGui::Checkbox("Multi light shader", &m_renderingSystem->m_sceneRenderer->m_multiLightShader);

                ImGui::SeparatorText("Draw calls");
                ImGui::Text("Total draw call count: %i", m_renderingSystem->m_sceneRenderer->m_renderingStatistics.DrawCallCount);

                ImGui::TreePop();
            }
            if (ImGui::TreeNode(ICON_FA_BUG " Debug renderer"))
            {
                ImGui::Checkbox("Draw AABBs ", &m_renderingSystem->m_sceneRenderer->m_AllowDebugDraw);

                ImGui::TreePop();
            }
        ImGui::End();

        IUserInterfaceElement::Render();
    }

    void RenderingOptions::Resize(int newWidth, int newHeight)
    {
    }

    void RenderingOptions::Update()
    {
        IUserInterfaceElement::Update();
    }
} // VEditor