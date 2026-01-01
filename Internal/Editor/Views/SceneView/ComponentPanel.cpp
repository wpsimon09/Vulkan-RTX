//
// Created by simon on 31/12/2025.
//

#include "ComponentPanel.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/ECS/Components/TransformComponent.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"

namespace VEditor {

ComponentPanel::ComponentPanel(ECS::ECSCoordinator& ecs)
    : m_ecs(ecs)
    , m_drawCommands(ecs)
{
}

void ComponentPanel::Render()
{
    ImGui::Begin(ICON_FA_CUBES "Components");

    if(m_selectedEntity != nullptr)
    {
        m_drawCommands.Draw(*m_selectedEntity);

        CreateAddComponentPopUp();

        ImVec2 cursorPos = ImGui::GetCursorPos();
        cursorPos.y += 10;
        ImGui::SetCursorPos(cursorPos);

        if(ImGui::Button(ICON_FA_CIRCLE_PLUS " Add component"))
        {
            ImGui::OpenPopup(V_EDITOR_ADD_COMPONENT_POP_UP_ID);
        }
    }
    else
    {
        ImGui::Text(ICON_FA_CIRCLE_EXCLAMATION " Nothing selected");
    }


    ImGui::End();
    IUserInterfaceElement::Render();
}

void ComponentPanel::Resize(int newWidth, int newHeight) {}

void ComponentPanel::Update()
{
    IUserInterfaceElement::Update();
}
void ComponentPanel::SetSelectedEntity(ECS::Entity* entity)
{
    m_selectedEntity = entity;
}
void ComponentPanel::CreateAddComponentPopUp()
{
    if(ImGui::BeginPopupContextItem(V_EDITOR_ADD_COMPONENT_POP_UP_ID))
    {
        AddComponentMenuItem<ECS::TransformComponent>(ICON_FA_MAP " Transformation");
        AddComponentMenuItem<ECS::MetadataComponent>(ICON_FA_CIRCLE_INFO "Metadata");

        ImGui::EndPopup();
    }
}

}  // namespace VEditor