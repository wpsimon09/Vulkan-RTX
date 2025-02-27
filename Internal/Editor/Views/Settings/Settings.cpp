//
// Created by wpsimon09 on 26/02/25.
//

#include "Settings.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include "Application/Client.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Camera/Camera.hpp"

namespace VEditor {
    Settings::Settings(Client& client, Editor* editor): m_client(client), m_editor(editor)
    {
    }

    void Settings::Render()
    {
        if (ImGui::BeginPopupModal(ICON_FA_GEARS" Settings"))
        {
            Utils::Logger::LogInfoClient("Opening settings window...");
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
            if (ImGui::Button("Close"))
            {
                Utils::Logger::LogInfoClient("Closing settings window...");
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
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

    void Settings::RenderEngineSettings()
    {
        if (ImGui::TreeNodeEx(ICON_FA_CAMERA "Camera"))
        {
            ImGui::DragFloat("Far plane:", &m_client.GetCamera().GetFarPlane(), 1.f, 40.0f, std::numeric_limits<float>::max());
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