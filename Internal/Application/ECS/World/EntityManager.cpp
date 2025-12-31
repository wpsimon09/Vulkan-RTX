//
// Created by simon on 30/12/2025.
//

#include "EntityManager.hpp"

namespace ECS {
EntityManager::EntityManager()
{
    for(ECS::Entity entity = 0; entity < ECS::MAX_ENTITIES; ++entity)
    {
        m_availableEntities.push(entity);
    }
}

ECS::Entity EntityManager::CreateEntity()
{
    assert(m_livingEntityCount < ECS::MAX_ENTITIES && "Too many entities alive in the scene");
    ECS::Entity id = m_availableEntities.front();
    m_availableEntities.pop();
    ++m_livingEntityCount;

    return id;
}
void EntityManager::DestroyEntity(ECS::Entity entity)
{
    assert(entity < ECS::MAX_ENTITIES && "Entity out of range");
    m_signatures[entity].reset();

    m_availableEntities.push(entity);
    --m_livingEntityCount;
}
void EntityManager::SetSignature(ECS::Entity entity, ECS::Signature signature)
{
    assert(entity < ECS::MAX_ENTITIES && "Entity out of range");
    m_signatures[entity] = signature;
}
Signature EntityManager::GetSignature(Entity entity)
{
    assert(entity < ECS::MAX_ENTITIES && "Entity out of range");
    return m_signatures[entity];
}
uint32_t EntityManager::GetLivingEntityCount()
{
    return m_livingEntityCount;
}


}  // namespace ECS