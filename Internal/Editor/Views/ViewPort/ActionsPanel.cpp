//
// Created by simon on 12/01/2026.
//

#include "ActionsPanel.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/World/World.hpp"

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
    if(ImGui::Button(ICON_FA_CIRCLE_PLUS "New"))
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
    if(ImGui::BeginPopup("AddPopUp"))
    {
        if(ImGui::BeginMenu(ICON_FA_SHAPES " Meshes"))
        {
            if(ImGui::MenuItem(ICON_FA_CUBES "Static mesh entity"))
            {
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu(ICON_FA_WAND_MAGIC_SPARKLES " Effects"))
        {
            if(ImGui::Selectable(ICON_FA_SMOG " Fog entity"))
            {
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu(ICON_FA_SUN " Lights"))
        {
            if(ImGui::Selectable(ICON_FA_CLOUD_SUN "Atmosphere entity"))
            {
            }
            if(ImGui::Selectable(ICON_FA_SUN " Sun light entity"))
            {
            }
            if(ImGui::Selectable(ICON_FA_LIGHTBULB " Point light entity"))
            {
            }
            if(ImGui::Selectable(ICON_FA_SQUARE " Area light entity"))
            {
            }
            if(ImGui::Selectable(ICON_FA_MOUNTAIN_SUN " Environment map entity"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

}  // namespace VEditor