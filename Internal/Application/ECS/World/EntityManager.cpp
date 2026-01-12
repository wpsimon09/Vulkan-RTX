//
// Created by simon on 30/12/2025.
//

#include "EntityManager.hpp"

namespace ECS {
/*
================================================================================
 EntityManager
 ----- chat gpt generated comment so that I understand what this does in future

 This class manages the lifetime and indexing of all ECS entities.

 Core idea:
   - Entities are just integer IDs.
   - All living entities are stored in a densely packed array for fast iteration.
   - A reverse-lookup table maps each entity ID to its position in that array.

 Data structures:

   m_livingEntities
       A tightly packed vector of all currently alive entity IDs.
       This is what systems iterate over every frame for cache-friendly access.

   m_entityToLivingEntity
       Maps: Entity ID -> index in m_livingEntities.
       This allows O(1) lookup of where any entity lives in the packed array.

   m_availableEntities
       A pool of unused entity IDs that can be recycled when entities are destroyed.

   m_signatures
       A bitmask per entity indicating which components it has.

 Why both a vector and an index map are needed:

   m_livingEntities tells us WHICH entities are alive.
   m_entityToLivingEntity tells us WHERE a specific entity is in that array.

   Example:
    m_livingEntities = [ 7, 3, 42, 19, 5 ]

    m_entityToLivingEntity[7]  = 0 // entity of id 7 is in the position 0 in the living entities vector
    m_entityToLivingEntity[3]  = 1 // entity of id 3 is in the position 1 in the living entities vector...
    m_entityToLivingEntity[42] = 2
    m_entityToLivingEntity[19] = 3
    m_entityToLivingEntity[5]  = 4

================================================================================
*/

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

    // map the ID of the crated entity to the one that will be in the living etities array
    // m_livingEntities[id] is the index of the created entity in the all entites array
    m_entityToLivingEntity[id] = m_livingEntities.size();
    m_livingEntities.push_back(id);

    ++m_livingEntityCount;
    return id;
}
void EntityManager::DestroyEntity(ECS::Entity entity)
{

    assert(entity < ECS::MAX_ENTITIES && "Entity out of range");

    m_signatures[entity].reset();

    // the same principle as with compoennts it replaces deleted entitiy with the entity that was last in the array
    size_t removedIndex = m_entityToLivingEntity[entity];
    size_t lastIndex    = m_livingEntities.size() - 1;

    ECS::Entity lastEntity = m_livingEntities[lastIndex];

    m_livingEntities[removedIndex]     = lastEntity;
    m_entityToLivingEntity[lastEntity] = removedIndex;

    m_livingEntities.pop_back();

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
const std::vector<Entity>& EntityManager::GetAliveEntities()
{
    return m_livingEntities;
}


}  // namespace ECS