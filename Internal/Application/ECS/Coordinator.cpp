//
// Created by simon on 31/12/2025.
//

#include "Coordinator.hpp"

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
void Coordinator::DestroyEntity(Entity entity)
{
    m_entityManager->DestroyEntity(entity);
    m_componentManager->OnEntityDestroyed(entity);
    m_systemManager->OnEntityDestroyed(entity);
}
}  // namespace ECS