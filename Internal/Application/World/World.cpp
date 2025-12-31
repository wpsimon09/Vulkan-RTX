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
}

ECS::ECSCoordinator& World::GetECS()
{
    return *m_ecsCoordinator;
}
}  // namespace ApplicationCore