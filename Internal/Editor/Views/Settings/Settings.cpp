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
#include "Application/Utils/Parsers/EngineDataParser.hpp"
#include "Editor/Editor.hpp"

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
        if (ImGui::TreeNodeEx(ICON_FA_CAMERA "Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
                ImGui::DragFloat("Camera speed:", &m_client.GetCamera().GetSpeed(), 1.f, 0.1, 20.0F);

                ImGui::DragFloat("Far plane:", &m_client.GetCamera().GetFarPlane(), 1.f, 40.0f, std::numeric_limits<float>::max());
                ImGui::DragFloat("Near plane:", &m_client.GetCamera().GetNearPlane(), 1.f, 40.0f, std::numeric_limits<float>::max());

                ImGui::SliderFloat("FOV", &m_client.GetCamera().GetFOV(), 0, 360);

            ImGui::TreePop();
        }
    }

    void Settings::RenderApplicationSettings()
    {
    }

    void Settings::RenderEditorSettings()
    {
    }
} // VEditor