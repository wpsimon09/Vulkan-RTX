//
// Created by simon on 31/12/2025.
//

#include "SystemManager.hpp"

namespace ECS {
/*
 * Checks if the entity whose signature was changed ( component was added ) can be usd in the
 * given system
 */
void SystemManager::OnEntitySignatureChanged(Entity entity, Signature signature)
{
    // go through every system and modify signatures of the given entites
    for(auto const& pair : m_systems)
    {
        auto const& type            = pair.first;
        auto const& system          = pair.second;
        auto const& systemSignature = m_signatures[type];

        if((signature & systemSignature) == systemSignature)
        {
            // this entity now has this signature and belongs to the specific system
            system->m_entities.insert(entity);
        }
        else
        {
            // this entity no longer has thsi signature and does not belong to this system
            system->m_entities.erase(entity);
        }
    }
}

void SystemManager::OnEntityDestroyed(Entity entity)
{
    // notify every system that the entity was destroyed
    for(auto const& pari : m_systems)
    {
        auto const& system = pari.second;
        system->m_entities.erase(entity);
    }
}
}  // namespace ECS