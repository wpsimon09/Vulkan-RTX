//
// Created by simon on 31/12/2025.
//

#include "ComponentPanel.hpp"

#include "IconsFontAwesome6.h"
#include "imgui.h"
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
    }
    else
    {
        ImGui::Text(ICON_FA_EXCLAMATION "Nothing selected");
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

}  // namespace VEditor