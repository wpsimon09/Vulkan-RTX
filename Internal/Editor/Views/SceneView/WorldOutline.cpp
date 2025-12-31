//
// Created by simon on 31/12/2025.
//

#include "WorldOutline.hpp"

#include "IconsFontAwesome6.h"
#include "Application/ECS/Coordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/World/World.hpp"

namespace VEditor {
WorldOutline::WorldOutline(ApplicationCore::World& world)
    : m_world(world)
{
    auto ecs = &m_world.GetECS();

    ecs->CreateEntityWithMetadata("test1", ICON_FA_TABLE, "");
    ecs->CreateEntityWithMetadata("test2", ICON_FA_2, "");
    ecs->CreateEntityWithMetadata("test3", ICON_FA_A, "");
    ecs->CreateEntityWithMetadata("test4", ICON_FA_ANCHOR_CIRCLE_CHECK, "");
    ecs->CreateEntityWithMetadata("test5", ICON_FA_4, "");
}
void WorldOutline::Render()
{
    auto ecs = &m_world.GetECS();

    ImGui::Begin(ICON_FA_MOUNTAIN " World outline");
    for(ECS::Entity entity = 0; entity < ecs->GetAllAliveEntities(); entity++)
    {
        auto& data = ecs->GetComponentFrom<ECS::MetadataComponent>(entity);
        ImGui::Text("Entity: %s", data.name.c_str());
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