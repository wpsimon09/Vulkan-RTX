//
// Created by simon on 31/12/2025.
//

#include "Coordinator.hpp"

#include "Components/MetadataComponent.hpp"
#include "System/SystemManager.hpp"
#include "World/ComponentManager.hpp"
#include "World/EntityManager.hpp"

namespace ECS {
Coordinator::Coordinator()
{
    m_componentManager = std::make_unique<ComponentManager>();
    m_entityManager    = std::make_unique<EntityManager>();
    m_systemManager    = std::make_unique<SystemManager>();
}

ECS::Entity Coordinator::CreateEntity()
{
    return m_entityManager->CreateEntity();
}
ECS::Entity Coordinator::CreateEntityWithMetadata(std::string name, char* icon, std::string tag)
{
    auto entity   = m_entityManager->CreateEntity();
    auto metaData = ECS::MetadataComponent{name, ICON_FA_INFO, 0, ""};
    m_componentManager->AddComponentTo<>(entity, metaData);

    return entity;
}

void Coordinator::DestroyEntity(Entity entity)
{
    m_entityManager->DestroyEntity(entity);
    m_componentManager->OnEntityDestroyed(entity);
    m_systemManager->OnEntityDestroyed(entity);
}
int Coordinator::GetAllAliveEntities()
{
    return m_entityManager->GetLivingEntityCount();
}
}  // namespace ECS