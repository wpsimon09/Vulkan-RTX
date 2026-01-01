//
// Created by simon on 31/12/2025.
//

#include "WorldOutline.hpp"

#include "ComponentPanel.hpp"
#include "IconsFontAwesome6.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/World/World.hpp"
#include "imgui.h"

namespace VEditor {
WorldOutline::WorldOutline(ApplicationCore::World& world)
    : m_world(world)
{
    auto ecs = &m_world.GetECS();

    m_uiChildren.push_back(std::make_unique<VEditor::ComponentPanel>(m_world.GetECS()));
    m_componentPanel = dynamic_cast<ComponentPanel*>(m_uiChildren.back().get());
}
void WorldOutline::Render()
{
    auto ecs = &m_world.GetECS();

    ImGui::Begin(ICON_FA_MOUNTAIN " World outline");
    for(ECS::Entity entity = 0; entity < ecs->GetAllAliveEntities(); entity++)
    {
        auto&       data  = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
        std::string label = data.icon + data.entityName;
        if(ImGui::Selectable(label.c_str(), m_selectedEntities.contains(entity) || m_selectedEntity == entity))
        {
            m_selectedEntity = entity;
            m_componentPanel->SetSelectedEntity(&m_selectedEntity);
        }
    }


    ImGui::End();

    IUserInterfaceElement::Render();
}

void WorldOutline::Resize(int newWidth, int newHeight) {}

void WorldOutline::Update()
{
    IUserInterfaceElement::Update();
}


}  // namespace VEditor