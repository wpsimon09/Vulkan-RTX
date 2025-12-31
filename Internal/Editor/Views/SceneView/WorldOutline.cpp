//
// Created by simon on 31/12/2025.
//

#include "WorldOutline.hpp"

#include "IconsFontAwesome6.h"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/World/World.hpp"

namespace VEditor {
WorldOutline::WorldOutline(ApplicationCore::World& world)
    : m_world(world)
{
    auto ecs = &m_world.GetECS();
}
void WorldOutline::Render()
{
    auto ecs = &m_world.GetECS();

    ImGui::Begin(ICON_FA_MOUNTAIN " World outline");
    for(ECS::Entity entity = 0; entity < ecs->GetAllAliveEntities(); entity++)
    {
        auto& data = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
        ImGui::Text("Entity: %s", data.entityName.c_str());
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