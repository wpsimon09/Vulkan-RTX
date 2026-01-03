//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_COMPONENTSTORAGE_HPP
#define VULKAN_RTX_COMPONENTSTORAGE_HPP
#include "Application/ECS/Types.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

#include <unordered_map>

namespace ECS {

class IComponentStorage
{
  public:
    virtual ~IComponentStorage()                  = default;
    virtual void OnEntityDestroyed(Entity entity) = 0;
};

/*
 * Class that acts as a storage for the components
 * each instance of the class acts as a storage for exactly one type of conmponent
 * [rigid body, rigid body, rigid body....]
 * [transform, transform, transform...]
 * [entity id : rigid body]
 */
template <typename T>
class ComponentStorage : public IComponentStorage
{
  public:
    void InsertData(Entity entity, T component)
    {
        VulkanUtils::RelaxedAssert(!m_entityToIndexMap.contains(entity), "This entity already has this component");
        size_t newIndex              = m_size;
        m_entityToIndexMap[entity]   = newIndex;
        m_indexToEntityMap[newIndex] = entity;
        m_componentsArray[newIndex]  = component;

        ++m_size;
    }

    void RemoveData(Entity entity)
    {
        assert(m_entityToIndexMap.contains(entity) && "This entity does not exist");
        size_t indexOfRemovedEntity             = m_entityToIndexMap[entity];
        size_t indexOfLastElement               = m_size - 1;
        m_componentsArray[indexOfRemovedEntity] = m_componentsArray[indexOfLastElement];

        // since component was removed we need move that data in the map and make entity point there
        Entity entityOfLastElement               = m_indexToEntityMap[indexOfLastElement];
        m_entityToIndexMap[entityOfLastElement]  = indexOfLastElement;
        m_indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        m_entityToIndexMap.erase(entity);
        m_indexToEntityMap.erase(indexOfLastElement);

        --m_size;
    }

    T& GetData(Entity entity)
    {
        assert(m_entityToIndexMap.contains(entity) && "This entity does not exist");
        return m_componentsArray[m_entityToIndexMap[entity]];
    }
    void OnEntityDestroyed(Entity entity) override
    {
        if(!m_entityToIndexMap.contains(entity))
        {
            RemoveData(entity);
        }
    }

  private:
    // components for hte given type (can be transformation, rigid body etc etc...)
    std::array<T, MAX_ENTITIES> m_componentsArray;

    // get specific compoents based on the entity idE
    std::unordered_map<Entity, size_t> m_entityToIndexMap;

    // suply component id located in (m_componentsArray) and get entity id
    std::unordered_map<size_t, Entity> m_indexToEntityMap;

    size_t m_size = 0;
};

}  // namespace ECS

#endif  //VULKAN_RTX_COMPONENTSTORAGE_HPP
