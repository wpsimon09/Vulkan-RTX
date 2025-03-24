//
// Created by wpsimon09 on 30/12/24.
//

#include "RenderingOptions.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/Rendering/Material/Material.hpp"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"

namespace VEditor {
    RenderingOptions::RenderingOptions(Renderer::RenderingSystem* renderingSystem)
    {
        m_renderingSystem = renderingSystem;
    }


    void RenderingOptions::Render()
    {
        ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS " Rendering options", &m_isOpen);

            ImGui::Checkbox("Fake ray-tracer ", &m_renderingSystem->m_isRayTracing);
            ImGui::Checkbox("Editor billboards ", &m_renderingSystem->m_renderContext.RenderBillboards);

            if (ImGui::TreeNode(ICON_FA_DRAW_POLYGON " Scene render"))
            {
                ImGui::Checkbox("Wire frame mode", &m_renderingSystem->m_renderContext.WireFrameRendering);

                ImGui::SeparatorText("Draw calls");
                ImGui::Text("Total draw call count: %i", m_renderingSystem->m_sceneRenderer->m_renderingStatistics.DrawCallCount);
                if (ImGui::Button("Show window with all draw calls"))
                {
                    m_openDrawCallListWindow = true;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(ICON_FA_BUG " Debug renderer"))
            {
                ImGui::Checkbox("Draw AABBs ", &m_renderingSystem->m_renderContext.RenderAABB);

                ImGui::TreePop();
            }
        ImGui::End();

        if (m_openDrawCallListWindow)
        {
            RenderDrawCallListWidndow(m_renderingSystem);
        }

        IUserInterfaceElement::Render();
    }

    void RenderingOptions::Resize(int newWidth, int newHeight)
    {

    }

    void RenderingOptions::Update()
    {
        IUserInterfaceElement::Update();
    }

    void RenderingOptions::RenderDrawCallListWidndow(Renderer::RenderingSystem* renderingSystem)
    {
        ImGui::Begin(ICON_FA_FILM " All draw calls");
        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        ImGui::BeginTable("Draw calls", 2, flags);
        {
            ImGui::TableSetupColumn("key");
            ImGui::TableSetupColumn("draw call info");
            ImGui::TableHeadersRow();

            for (auto & drawCall : m_renderingSystem->m_renderContext.drawCalls)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%i", drawCall.first);

                ImGui::TableSetColumnIndex(1);

                ImGui::SeparatorText("Buffer info");

                ImGui::Text("Index Count: %u", drawCall.second.indexCount);
                ImGui::Text("First Index: %u", drawCall.second.firstIndex);
                ImGui::Text("Instance Count: %u", drawCall.second.instanceCount);
                ImGui::Text("Vertex buffer ID: %i", drawCall.second.vertexData->BufferID);
                ImGui::Text("Index  buffer ID: %i", drawCall.second.indexData->BufferID);

                ImGui::SeparatorText("Appearance info");

                // Print material name (if exists)
                if (drawCall.second.material)
                {
                    ImGui::Text("Material: %s", drawCall.second.material->GetMaterialName().c_str());
                }

                // Print effect name (if exists)
                if (drawCall.second.effect)
                {
                    ImGui::Text("Effect: %s", drawCall.second.effect->GetName().c_str());
                    ImGui::Text("Effect ID: %i", drawCall.second.effect->GetID());
                }

                // Print position
                ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                            drawCall.second.position.x, drawCall.second.position.y, drawCall.second.position.z);

            }
            ImGui::EndTable();
        }


        if (ImGui::Button("Close"))
        {
            m_openDrawCallListWindow = false;
        }

        ImGui::End();

    }


} // VEditor