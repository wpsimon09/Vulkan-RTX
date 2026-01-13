//
// Created by simon on 12/01/2026.
//

#include "ActionsPanel.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/World/World.hpp"
#include "Application/World/WorldCreatorHelper.hpp"

namespace VEditor {
ActionsPanel::ActionsPanel(ApplicationCore::World& world)
    : m_world(world)
    , m_ecs(world.GetECS())
    , IUserInterfaceElement()
{
}
void ActionsPanel::Render()
{
    bool open = true;
    ImGui::BeginChild("Actions panel", ImVec2(ImGui::GetContentRegionAvail().x * 0.2, 40));
    if(ImGui::Button(ICON_FA_CIRCLE_PLUS " New"))
    {
        ImGui::OpenPopup(ADD_POP_UP_ID);
    }
    RenderAddNewPopUp();

    ImGui::EndChild();
    IUserInterfaceElement::Render();
}
void ActionsPanel::Resize(int newWidth, int newHeight) {}

void ActionsPanel::Update()
{
    IUserInterfaceElement::Update();
}
void ActionsPanel::RenderAddPopUp() {}

void ActionsPanel::RenderAddNewPopUp()
{
    if(ImGui::BeginPopup(ADD_POP_UP_ID))
    {
        if(ImGui::BeginMenu(ICON_FA_SHAPES " Meshes"))
        {
            if(ImGui::MenuItem(ICON_FA_CUBES "Static mesh entity"))
            {
                ApplicationCore::AddStaticMeshEntity(m_world);
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu(ICON_FA_WAND_MAGIC_SPARKLES " Effects"))
        {
            if(ImGui::Selectable(ICON_FA_SMOG " Fog entity"))
            {
                ApplicationCore::AddFogEntity(m_world);
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu(ICON_FA_SUN " Lights"))
        {
            if(ImGui::Selectable(ICON_FA_CLOUD_SUN "Atmosphere entity"))
            {
                ApplicationCore::AddAtmosphereEntity(m_world);
            }
            if(ImGui::Selectable(ICON_FA_SUN " Sun light entity"))
            {
                ApplicationCore::AddSunLightEntity(m_world);
            }
            if(ImGui::Selectable(ICON_FA_LIGHTBULB " Point light entity"))
            {
                ApplicationCore::AddPointLightEntity(m_world);
            }
            if(ImGui::Selectable(ICON_FA_SQUARE " Area light entity"))
            {
                ApplicationCore::AddAreaLightEntity(m_world);
            }
            if(ImGui::Selectable(ICON_FA_MOUNTAIN_SUN " Environment map entity"))
            {
                ApplicationCore::AddEnvironmentLightEntity(m_world);
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

}  // namespace VEditor