//
// Created by wpsimon09 on 26/02/25.
//

#include "Settings.hpp"

#include <future>
#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/Enums/ClientEnums.hpp"
#include "VulkanRtx.hpp"
#include "Application/Client.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Utils/ApplicationUtils.hpp"
#include "Application/Utils/Parsers/EngineDataParser.hpp"
#include "Editor/Editor.hpp"
#include "Editor/EditorOptions.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/EngineOptions.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Application/ApplicationState/ApplicationState.hpp"

namespace VEditor {
Settings::Settings(Client& client, Editor* editor)
    : m_client(client)
    , m_editor(editor)
{
    m_isOpen = false;
}

void Settings::Render()
{
    if(m_isOpen)
    {
        ImGui::OpenPopup(ICON_FA_GEARS " Settings");

        if(ImGui::BeginPopupModal(ICON_FA_GEARS " Settings"))
        {
            if(ImGui::BeginTabBar("Settings"))
            {

                if(ImGui::BeginTabItem("Engine"))
                {
                    RenderEngineSettings();
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Rendering"))
                {
                    RenderRenderingSettings();
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Editor"))
                {
                    RenderEditorSettings();
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("Application"))
                {
                    RenderApplicationSettings();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 50);
            ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 30);
            if(ImGui::Button("Close"))
            {
                std::async([this]() { ApplicationCore::SaveConfig(m_client, m_editor->m_uiContext); });
                m_isOpen = false;
            }

            ImGui::EndPopup();
        }
    }

    IUserInterfaceElement::Render();
}

void Settings::Resize(int newWidth, int newHeight) {}

void Settings::Update()
{
    IUserInterfaceElement::Update();
}

void Settings::Open()
{
    m_isOpen = true;
    ImGui::OpenPopup(ICON_FA_GEARS " Settings");
}

void Settings::RenderEngineSettings()
{
    int v = GlobalVariables::EngineOptions::VertexBufferChunkSize;
    ImGui::InputInt("Vertex buffer chunk size", &v, 8, 1024);
    GlobalVariables::EngineOptions::VertexBufferChunkSize = v;

    int i = GlobalVariables::EngineOptions::IndexBufferChunkSize;
    ImGui::InputInt("Index buffer chunk size", &i, 8, 1024);
    GlobalVariables::EngineOptions::IndexBufferChunkSize = i;
}

void Settings::RenderApplicationSettings()
{
    if(ImGui::TreeNodeEx(ICON_FA_CAMERA "Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Camera speed:", &m_client.GetCamera().GetSpeed(), 1.f, 0.1, 20.0F);

        ImGui::DragFloat("Far plane:", &m_client.GetCamera().GetFarPlane(), 1.f, 40.0f, std::numeric_limits<float>::max());
        ImGui::DragFloat("Near plane:", &m_client.GetCamera().GetNearPlane(), 1.f, 0.2f, std::numeric_limits<float>::max());

        if(ImGui::TreeNode(ICON_FA_CAMERA "Camera"))
        {
            if(ImGui::BeginCombo(ICON_FA_CAMERA_RETRO "Type", m_cameraOption[m_client.GetCamera().GetCurrentCameraType()]))
            {

                for(int i = 0; i < m_cameraOption.size(); i++)
                {
                    if(ImGui::Selectable(m_cameraOption[i], i == m_client.GetCamera().GetCurrentCameraType()))
                    {
                        m_client.GetCamera().GetCurrentCameraType() = (ECameraTypes)i;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SliderFloat("FOV", &m_client.GetCamera().GetFOV(), 1, 360);
            ImGui::Text("FOV Radians: %f", m_client.GetCamera().GetFOVRad());

            ImGui::DragFloat("Focal length:", &m_client.GetCamera().GetFocalLength(), 0.2f);

            ImGui::DragFloat("Aparature size ", &m_client.GetCamera().GetAparatureSize(), 0.1f, 0.0f);

            ImGui::DragFloat("Image plane distance ", &m_client.GetCamera().GetImagePlaneDistance(), 0.1, 0.0f);

            ImGui::DragFloat("Defocuse strength", &m_client.GetCamera().GetDefocuseStrength(), 0.1f, 0.0);

            ImGui::TreePop();
        }

        m_client.GetCamera().Recalculate();

        ImGui::TreePop();
    }
}

void Settings::RenderEditorSettings()
{
    if(ImGui::BeginCombo("Theme", ThemeToString(GlobalVariables::EditorOptions::Theme).c_str()))
    {
        for(int i = 0; i < 2; i++)
        {
            bool isSelected = (static_cast<int>(GlobalVariables::EditorOptions::Theme) == i);
            if(ImGui::Selectable(ThemeToString(static_cast<ETheme>(i)).c_str()))
            {
                GlobalVariables::EditorOptions::Theme = static_cast<ETheme>(i);
            }
            if(isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if(GlobalVariables::EditorOptions::Theme == ETheme::Dark)
    {
        m_editor->m_uiContext.SetColourThemePabloDark();
    }
    else
    {
        m_editor->m_uiContext.SetColourThemePabloLight();
    }
}

void Settings::RenderRenderingSettings()
{
    ImGui::Checkbox("Frustrum culling: ", &GlobalVariables::RenderingOptions::EnableFrustrumCulling);
    ImGui::Checkbox("MSAA Enabled: ", &GlobalState::MSAA);
    ImGui::Checkbox("Depth pre pass: ", &GlobalVariables::RenderingOptions::PreformDepthPrePass);

    if(GlobalState::MSAA)
    {
        if(ImGui::BeginCombo("Number of samples", std::to_string(GlobalVariables::RenderingOptions::MSAASamples).c_str()))
        {
            static const int sampleOptions[] = {1, 2, 4, 8, 16, 32, 64};  // Common MSAA sample values
            for(int samples : sampleOptions)
            {
                bool isSelected = (GlobalVariables::RenderingOptions::MSAASamples == samples);
                if(ImGui::Selectable(std::to_string(samples).c_str(), isSelected))
                {
                    GlobalVariables::RenderingOptions::MSAASamples = samples;
                }

                if(isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
    }

    ImGui::SliderFloat("Outline width", &GlobalVariables::RenderingOptions::OutlineWidth, 0.01f, 1.f);

    if(ImGui::TreeNode("Ray tracing"))
    {
        auto& applicationState = m_client.GetApplicationState();
        ImGui::SliderInt("Reccursion depth", &GlobalVariables::RenderingOptions::MaxRecursionDepth, 1, 20);
        ImGui::SliderInt("Rays per pixel", &GlobalVariables::RenderingOptions::RaysPerPixel, 1, 20);

        if(ImGui::TreeNode("RT Ambient occlusion"))
        {
            ImGui::DragFloat("Radius", &applicationState.GetAoOcclusionParameters().radius, 0.2);
            ImGui::DragFloat("Sample count ", &applicationState.GetAoOcclusionParameters().sampleCount, 1);
            ImGui::DragFloat("Intensity ", &applicationState.GetAoOcclusionParameters().strenght, 1);

            ImGui::TreePop();
        }

        if(ImGui::TreeNode("De-noising"))
        {
            ImGui::Checkbox("Denoise", &applicationState.m_denoise);

            if(applicationState.m_denoise)
            {

                ImGui::SliderFloat("Spacial", &applicationState.GetBilateralFilaterParameters().sigma, 0.001, 300.0);
                ImGui::SliderFloat("Range", &applicationState.GetBilateralFilaterParameters().BSigma, 1, 300.0);
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    ImGui::Checkbox("Enable post processing ", &m_client.GetApplicationState().m_enablePostProcessing);
    if(m_client.GetApplicationState().m_enablePostProcessing)
    {

        if(ImGui::TreeNode("Tone mapping options"))
        {

            auto& applicationState = m_client.GetApplicationState();


            if(ImGui::BeginCombo("Tone mapping curve",
                                 m_toneMappingCurves[(EToneMappingCurve)applicationState.GetToneMappingParameters().curve]))
            {
                int j = 0;
                for(auto& curve : m_toneMappingCurves)
                {
                    if(ImGui::Selectable(m_toneMappingCurves[j],
                                         (EToneMappingCurve)applicationState.GetToneMappingParameters().curve == (EToneMappingCurve)j))
                    {
                        applicationState.GetToneMappingParameters().curve = (EToneMappingCurve)j;
                    }
                    j++;
                }
                ImGui::EndCombo();
            }

            if(ImGui::TreeNode("Luminance"))
            {
                ImGui::SeparatorText("Histogram");

                ImGui::SliderFloat("Minimal logaritmic luminance",
                                   &applicationState.GetLuminanceHistogramParameters().minLogLuminance, -95, 0);

                ImGui::SliderFloat("Maximal logaritmic luminance",
                                   &applicationState.GetLuminanceHistogramParameters().maxLogLuminance, -50, 50);

                float luminanceRange = applicationState.GetLuminanceHistogramParameters().CalculateLuminanceRange();
                ImGui::Text("Luminance range: %f", luminanceRange);
                ImGui::Text("One over range: %f", applicationState.GetLuminanceHistogramParameters().oneOverLogLuminanceRange);

                ImGui::SeparatorText("Average");

                ImGui::SliderFloat("Tau", &applicationState.GetLuminanceAverageParameters().tau, 0.0, 5.0);


                ImGui::TreePop();
            }

            ImGui::TreePop();
        }


        if(ImGui::TreeNode("Lens flare"))
        {
            auto& applicationState = m_client.GetApplicationState();

            ImGui::SliderFloat("Strength", &applicationState.GetLensFlareParameters().lensFlareStrength, 0.1f, 100.0f);
            ImGui::SliderFloat("F1 Strength:", &applicationState.GetLensFlareParameters().f1Strength, 0.1f, 100.0f);
            ImGui::SliderFloat("F2 Strength:", &applicationState.GetLensFlareParameters().f2Strength, 0.1f, 100.0f);
            ImGui::SliderFloat("F3 Strength:", &applicationState.GetLensFlareParameters().f3Strength, 0.1f, 100.0f);

            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Bloom"))
        {
            auto& bloomSettings = m_client.GetApplicationState().GetBloomSettings();

            ImGui::Checkbox("Lens dirt", reinterpret_cast<bool*>(&bloomSettings.dirtTexture));
            if(bloomSettings.dirtTexture)
            {
                ImGui::DragFloat("Dirt texture intensity", &bloomSettings.dirstIntensity, 0.1, 0.0);
            }

            ImGui::SliderFloat("Bloom strength", &bloomSettings.bloomStrenght, 0.0, 1.0);
            ImGui::SliderFloat("Filter radius", &bloomSettings.filterRadius, 0.001, 1.0);

            ImGui::TreePop();
        }
    }
}
}  // namespace VEditor