//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_WORLD_HPP
#define VULKAN_RTX_WORLD_HPP
#include "Application/ECS/Entity.hpp"


#include <queue>

namespace ECS {

class EntityManager
{
  public:
    EntityManager();

    Entity CreateEntity();

    void DestroyEntity(Entity entity);

    void SetSignature(Entity entity, Signature signature);

    Signature GetSignature(Entity entity);

  private:
    /*
   * Stores unissued IDs of the entities
   */
    std::queue<Entity> m_availableEntities;
    /*
   * Contains bit fields for each entity
   * we decompose each bit of the 32-bit number where each bit is representing weather given
   * entity has the given component
   * Example: 1001
   * - transform component (y) rigid body (n) light (n) mesh (y)
   */
    std::array<Signature, MAX_ENTITIES> m_signatures;

    uint32_t m_livingEntityCount;
};

}  // namespace ECS

#endif  //VULKAN_RTX_WORLD_HPP
