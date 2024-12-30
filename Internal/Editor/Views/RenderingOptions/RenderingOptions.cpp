//
// Created by wpsimon09 on 30/12/24.
//

#include "RenderingOptions.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

namespace VEditor {
    RenderingOptions::RenderingOptions(const Renderer::RenderingSystem* renderingSystem)
    {
        m_renderingSystem = renderingSystem;
    }


    void RenderingOptions::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS " Rendering options", &m_isOpen);

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