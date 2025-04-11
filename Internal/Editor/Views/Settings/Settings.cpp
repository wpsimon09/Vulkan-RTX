//
// Created by wpsimon09 on 26/02/25.
//

#include "Settings.hpp"

#include <future>
#include <IconsFontAwesome6.h>
#include <imgui.h>

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

namespace VEditor {
    Settings::Settings(Client& client, Editor* editor): m_client(client), m_editor(editor)
    {
        m_isOpen = false;
    }

    void Settings::Render()
    {
        if (m_isOpen)
        {
            ImGui::OpenPopup(ICON_FA_GEARS" Settings");

            if (ImGui::BeginPopupModal(ICON_FA_GEARS" Settings"))
            {
                if (ImGui::BeginTabBar("Settings"))
                {

                    if (ImGui::BeginTabItem("Engine"))
                    {
                        RenderEngineSettings();
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Rendering"))
                    {
                        RenderRenderingSettings();
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Editor"))
                    {
                        RenderEditorSettings();
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Application"))
                    {
                        RenderApplicationSettings();
                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }

                ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 50);
                ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 30);
                if (ImGui::Button("Close"))
                {
                    std::async([this](){
                        ApplicationCore::SaveConfig(m_client, m_editor->m_uiContext);
                    });
                    m_isOpen = false;
                }

                ImGui::EndPopup();
            }
        }

        IUserInterfaceElement::Render();
    }

    void Settings::Resize(int newWidth, int newHeight)
    {
    }

    void Settings::Update()
    {
        IUserInterfaceElement::Update();
    }

    void Settings::Open()
    {
        m_isOpen = true;
        ImGui::OpenPopup(ICON_FA_GEARS" Settings");
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
        if (ImGui::TreeNodeEx(ICON_FA_CAMERA "Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat("Camera speed:", &m_client.GetCamera().GetSpeed(), 1.f, 0.1, 20.0F);

            ImGui::DragFloat("Far plane:", &m_client.GetCamera().GetFarPlane(), 1.f, 40.0f, std::numeric_limits<float>::max());
            ImGui::DragFloat("Near plane:", &m_client.GetCamera().GetNearPlane(), 1.f, 0.2f, std::numeric_limits<float>::max());

            ImGui::SliderFloat("FOV", &m_client.GetCamera().GetFOV(), 1, 360);

            m_client.GetCamera().Recalculate();

            ImGui::TreePop();
        }
    }

    void Settings::RenderEditorSettings()
    {
        if (ImGui::BeginCombo("Theme", ThemeToString(GlobalVariables::EditorOptions::Theme).c_str()))
        {
            for (int i = 0; i<2; i++)
            {
                bool isSelected = (static_cast<int>(GlobalVariables::EditorOptions::Theme) == i);
                if (ImGui::Selectable(ThemeToString(static_cast<ETheme>(i)).c_str()))
                {
                    GlobalVariables::EditorOptions::Theme = static_cast<ETheme>(i);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (GlobalVariables::EditorOptions::Theme == ETheme::Dark)
        {
            m_editor->m_uiContext.SetColourThemePabloDark();
        }else
        {
            m_editor->m_uiContext.SetColourThemePabloLight();
        }
    }

    void Settings::RenderRenderingSettings()
    {
        ImGui::Checkbox("Frustrum culling: ", &GlobalVariables::RenderingOptions::EnableFrustrumCulling);
        ImGui::Checkbox("MSAA Enabled: ", &GlobalState::MSAA);
        ImGui::Checkbox("Depth pre pass: ", &GlobalVariables::RenderingOptions::PreformDepthPrePass);

        if (GlobalState::MSAA)
        {
            if (ImGui::BeginCombo("Number of samples", std::to_string(GlobalVariables::RenderingOptions::MSAASamples).c_str()))
            {
                static const int sampleOptions[] = {1, 2, 4, 8, 16, 32, 64}; // Common MSAA sample values
                for (int samples : sampleOptions)
                {
                    bool isSelected = (GlobalVariables::RenderingOptions::MSAASamples == samples);
                    if (ImGui::Selectable(std::to_string(samples).c_str(), isSelected))
                    {
                        GlobalVariables::RenderingOptions::MSAASamples = samples;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                                    }

                ImGui::EndCombo();
            }
        }

        ImGui::SliderFloat("Outline width", &GlobalVariables::RenderingOptions::OutlineWidth, 0.01f, 1.f);
    }
} // VEditor