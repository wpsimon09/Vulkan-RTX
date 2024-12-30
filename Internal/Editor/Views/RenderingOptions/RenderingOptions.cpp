//
// Created by wpsimon09 on 30/12/24.
//

#include "RenderingOptions.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"

namespace VEditor {
    RenderingOptions::RenderingOptions(const Renderer::RenderingSystem* renderingSystem)
    {
        m_renderingSystem = renderingSystem;
    }


    void RenderingOptions::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS " Rendering options", &m_isOpen);
            if (ImGui::TreeNode(ICON_FA_DRAW_POLYGON " SceneRenderer"))
            {

                ImGui::Checkbox("Draw debug lines", &m_renderingSystem->m_sceneRenderer->m_WireFrame);

                ImGui::TreePop();
            }
            if (ImGui::TreeNode(ICON_FA_BUG " Debug renderer"))
            {
                ImGui::Checkbox("Draw debug lines", &m_renderingSystem->m_sceneRenderer->m_AllowDebugDraw);

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