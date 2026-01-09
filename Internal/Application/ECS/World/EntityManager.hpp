//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_ENTITYMANAGER_HPP
#define VULKAN_RTX_ENTITYMANAGER_HPP

#include "Application/ECS/Types.hpp"
#include <queue>
#include <unordered_map>

namespace ECS {

class EntityManager
{
  public:
    EntityManager();

    Entity CreateEntity();

    void DestroyEntity(Entity entity);

    void SetSignature(Entity entity, Signature signature);

    Signature GetSignature(Entity entity);

    uint32_t GetLivingEntityCount();

    const std::vector<Entity>& GetAliveEntities();

  private:
    /*
   * Stores unissued IDs of the entities
   */
    std::queue<Entity> m_availableEntities{};

    /*
   * Stores all alive entities
   */
    std::vector<Entity> m_livingEntities{};

    /*
   * Contains bit fields for each entity
   * we decompose each bit of the 32-bit number where each bit is representing weather given
   * entity has the given component
   * Example: 1001
   * - transform component (y) rigid body (n) light (n) mesh (y)
   */
    std::array<Signature, MAX_ENTITIES> m_signatures{};

    std::array<Entity, MAX_ENTITIES> m_entityToLivingEntity{};

    uint32_t m_livingEntityCount{};
};

}  // namespace ECS

#endif  //VULKAN_RTX_ENTITYMANAGER_HPP
