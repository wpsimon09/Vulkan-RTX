//
// Created by simon on 31/12/2025.
//

#include "World.hpp"

#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/MetadataComponent.hpp"
#include "Application/ECS/Components/TransformComponent.hpp"

namespace ApplicationCore {
World::World(ApplicationState& applicationState)
    : m_applicationState(applicationState)
{
    m_ecsCoordinator = std::make_unique<ECS::ECSCoordinator>();
}

void World::RegisterAll()
{
    m_ecsCoordinator->RegisterComponent<ECS::TransformComponent>();
    m_ecsCoordinator->RegisterComponent<ECS::MetadataComponent>();

    m_ecsCoordinator->CreateEntityWithMetadata("test1", ICON_FA_TABLE, "");
    m_ecsCoordinator->CreateEntityWithMetadata("test2", ICON_FA_2, "");
    m_ecsCoordinator->CreateEntityWithMetadata("test3", ICON_FA_A, "");
    m_ecsCoordinator->CreateEntityWithMetadata("test4", ICON_FA_ANCHOR_CIRCLE_CHECK, "");

    auto e = m_ecsCoordinator->CreateEntityWithMetadata("test5", ICON_FA_4, "");
    m_ecsCoordinator->AddComponentTo(e, ECS::TransformComponent());
}

ECS::ECSCoordinator& World::GetECS()
{
    return *m_ecsCoordinator;
}
}  // namespace ApplicationCore