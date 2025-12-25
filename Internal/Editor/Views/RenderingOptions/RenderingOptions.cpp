//
// Created by wpsimon09 on 30/12/24.
//

#include "RenderingOptions.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/ApplicationState/ApplicationState.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Vulkan/Renderer/Renderers/Frame.hpp"
#include "Vulkan/Renderer/Renderers/ForwardRenderer.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Application/Rendering/Camera/Camera.hpp"

namespace VEditor {
RenderingOptions::RenderingOptions(ApplicationCore::ApplicationState& applicationState,
                                   ApplicationCore::Scene&            scene,
                                   Renderer::Frame*                   renderingSystem)
    : m_scene(scene)
    , m_applicationState(applicationState)
{
    m_renderingSystem = renderingSystem;
}


void RenderingOptions::Render()
{
    ImGui::Begin(ICON_FA_BOOK_JOURNAL_WHILLS " Rendering options", &m_isOpen);

    ImGui::Text("Camera pos: x: %f , y:%f , z:%f", m_scene.GetCamera().GetPosition().x,
                m_scene.GetCamera().GetPosition().y, m_scene.GetCamera().GetPosition().z);
    if(ImGui::Button("Rebuild AS"))
    {
        auto input = m_scene.GetBLASInputs();
    }

    if(ImGui::TreeNode(ICON_FA_ARROWS_LEFT_RIGHT_TO_LINE " RTX"))
    {
        ImGui::Checkbox("RTX ", &m_renderingSystem->m_isRayTracing);
        ImGui::Checkbox("SER (Shader exectution reordering)",
                        reinterpret_cast<bool*>(&m_applicationState.GetGlobalRenderingInfo2().renderingFeatures.z));
        ImGui::Checkbox("Accumulate frames ", &m_applicationState.m_accumulateFrames);

        ImGui::TreePop();
    }


    if(ImGui::TreeNode(ICON_FA_DRAW_POLYGON " Scene render"))
    {
        ImGui::Checkbox("Editor billboards ", &m_renderingSystem->m_renderContext.RenderBillboards);
        ImGui::Checkbox("Wire frame mode", &m_renderingSystem->m_renderContext.WireFrameRendering);
        ImGui::Checkbox("Composite", &m_applicationState.m_composite);
        if(ImGui::TreeNode("Ray traced reflections"))
        {
            ImGui::Checkbox("RayTraced reflections", &m_applicationState.m_rayTracedReflections);
            ImGui::Checkbox("Accumulate##1", reinterpret_cast<bool*>(&m_applicationState.GetReflectionsParameters().accumulate));
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Ambient occlusion "))
        {
            ImGui::Checkbox("Ambient occlusion ", &m_applicationState.m_ambientOcclusion);
            ImGui::Checkbox("Accumulate##2", reinterpret_cast<bool*>(&m_applicationState.GetAoOcclusionParameters().accumulate));

            ImGui::TreePop();
        }

        ImGui::SeparatorText("Draw calls");
        ImGui::Text("Total draw call count: %i", m_renderingSystem->m_forwardRenderer->m_renderingStatistics.DrawCallCount);
        if(ImGui::Button("Show window with all draw calls"))
        {
            m_openDrawCallListWindow = true;
        }
        if(ImGui::Button("Show light info"))
        {
            m_openLightInfoLigt = true;
        }
        ImGui::Checkbox("Lens flare", &m_renderingSystem->m_postProcessingContext.lensFlareEffect);
        ImGui::Checkbox("Bloom", &m_renderingSystem->m_postProcessingContext.bloomEffect);


        ImGui::Text("Number of frames: %lu", m_renderingSystem->m_frameCount);
        ImGui::Text("Accumulated frames: %iu", m_renderingSystem->m_accumulatedFramesCount);

        ImGui::TreePop();
    }
    if(ImGui::TreeNode(ICON_FA_BUG " Debug renderer"))
    {
        ImGui::Checkbox("Draw AABBs ", &m_renderingSystem->m_renderContext.RenderAABB);

        ImGui::TreePop();
    }
    ImGui::End();

    if(m_openDrawCallListWindow)
    {
        RenderDrawCallListWidndow(m_renderingSystem);
    }

    if(m_openLightInfoLigt)
    {
        RenderLightInfoWindow(m_renderingSystem);
    }

    IUserInterfaceElement::Render();
}

void RenderingOptions::Resize(int newWidth, int newHeight) {}

void RenderingOptions::Update()
{
    IUserInterfaceElement::Update();
}

void RenderingOptions::RenderDrawCallListWidndow(Renderer::Frame* renderingSystem)
{
    ImGui::Begin(ICON_FA_FILM " All draw calls");
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImGui::BeginTable("Draw calls", 2, flags);
    {
        ImGui::TableSetupColumn("key");
        ImGui::TableSetupColumn("draw call info");
        ImGui::TableHeadersRow();

        for(auto& drawCall : m_renderingSystem->m_renderContext.drawCalls)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%lu", drawCall.first);

            ImGui::TableSetColumnIndex(1);

            if(!drawCall.second.postProcessingEffect)
            {

                ImGui::SeparatorText("Buffer info");
                ImGui::Text("Index Count: %u", drawCall.second.indexCount);
                ImGui::Text("First Index: %u", drawCall.second.firstIndex);
                ImGui::Text("Instance Count: %u", drawCall.second.instanceCount);
                ImGui::Text("Vertex buffer ID: %i", drawCall.second.vertexData->BufferID);
                ImGui::Text("Index  buffer ID: %i", drawCall.second.indexData->BufferID);
                ImGui::Text("Material ID: %i", drawCall.second.materialIndex);

                ImGui::SeparatorText("Appearance info");

                // Print material name (if exists)
                if(drawCall.second.material)
                {
                    ImGui::Text("Material: %s", drawCall.second.material->GetMaterialName().c_str());
                }

                // Print effect name (if exists)
                if(drawCall.second.effect)
                {
                    ImGui::Text("Effect ID: %s", m_forwardRendererEffects[drawCall.second.effect]);
                }

                // Print position
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", drawCall.second.position.x, drawCall.second.position.y,
                            drawCall.second.position.z);
            }
            else
            {
                ImGui::Text("Post processing volume draw call");
            }
        }
        ImGui::EndTable();
    }


    if(ImGui::Button("Close"))
    {
        m_openDrawCallListWindow = false;
    }

    ImGui::End();
}

void RenderingOptions::RenderLightInfoWindow(Renderer::Frame* renderingSystem)
{
    ImGui::Begin(ICON_FA_BOLT_LIGHTNING " Light info");

    ImGui::SliderFloat("Ambient strength", &m_scene.GetSceneLightInfo().ambientStrenght, 0.0f, 1.0f);

    if(ImGui::Button("Close"))
    {
        m_openLightInfoLigt = false;
    }

    ImGui::End();
}
}  // namespace VEditor