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
        /*
         * m_componentsArray -> array of all components
         * [ECS::Transfrom, ECS::Transfrom, ECS::Transform]
         * --------------------
         * m_indexToEntity map --> maps which components belong to what entity
         * [componentId : key, entityId: value]
         * --------------------
         * m_entityToIndexMap -> maps which entity goes to which index in the components array
         * [entityId : key, componentId : value]
         * --------------------
         * the `componentId` is allways index to the m_componentsArray !
        */
        assert(m_entityToIndexMap.contains(entity) && "This entity does not exist");

        // get the index of a component that the entity belongs to
        size_t componentIndexOfRemovedEntity = m_entityToIndexMap[entity];

        // get the index of the last component in the components array
        size_t indexOfLastElement = m_size - 1;

        // move last component in place of removed entity
        m_componentsArray[componentIndexOfRemovedEntity] = m_componentsArray[indexOfLastElement];

        // get the entity that belonged to the last element which was moved in place of deleted entity
        Entity entityOfLastElement = m_indexToEntityMap[indexOfLastElement];

        // move the removed entity to the place of the last entity.
        // make the last entity point to the index of the compoennt from the entity which was removed and moved to the end        m_entityToIndexMap[entityOfLastElement] = componentIndexOfRemovedEntity;
        // make the component of the removed entity point to the entity at the end of the array which is now the deleted entity
        m_entityToIndexMap[entityOfLastElement]           = componentIndexOfRemovedEntity;
        m_indexToEntityMap[componentIndexOfRemovedEntity] = entityOfLastElement;
        ;

        m_entityToIndexMap.erase(entity);
        m_indexToEntityMap.erase(indexOfLastElement);

        --m_size;
    }

    T& GetData(Entity entity)
    {
        assert(m_entityToIndexMap.contains(entity) && "This entity does not exist");
        return m_componentsArray[m_entityToIndexMap[entity]];
    }

    void SetData(T& data, Entity entity) { m_componentsArray[m_entityToIndexMap.at(entity)] = data; }

    void OnEntityDestroyed(Entity entity) override
    {
        if(m_entityToIndexMap.contains(entity))
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
