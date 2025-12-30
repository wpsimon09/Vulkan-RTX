//
// Created by simon on 30/12/2025.
//

#ifndef VULKAN_RTX_COMPONENTMANAGER_HPP
#define VULKAN_RTX_COMPONENTMANAGER_HPP
#include "ComponentStorage.hpp"
#include "Application/ECS/Types.hpp"
#include "Application/Logger/Logger.hpp"


#include <memory>
#include <unordered_map>

namespace ECS {
class IComponentStorage;

/**
 * Used to dinamically register new components to the ECS
 * it has to be called and filled with all the components that i will have
 */
class ComponentManager
{
  public:
    template <typename T>
    void RegisterComponent()
    {
        const char* typeName = typeid(T).name();
        assert(!m_componentsStorages.contains(typeName) && "Component already registered !");
        Utils::Logger::LogInfoClient("ECS::Component " + std::string(typeName) + " registered");

        // the component types array is used to create signature to signalize which entity has which components
        m_componentTypes.insert({typeName, m_nextComponentType});
        m_componentsStorages.insert({typeName, std::make_shared<ComponentStorage<T>>()});

        ++m_nextComponentType;
    }

    /*
     * Retrieves component type for the signature
     * 0 - is transform, 2 - rigid body,  4 - material etc etc....
     */
    template <typename T>
    ComponentType GetComponentType()
    {
        const char* typeName = typeid(T).name();
        assert(m_componentTypes.contains(typeName) && "Component not registered !");

        return m_componentTypes[typeName];
    }

    /*
     * Adds component to the entity
     **/
    template <typename T>
    void AddComponentTo(Entity entity, T component)
    {
        GetStorageForComponentType<T>()->InsertData(entity, component);
    }

    /*
     * Removes component from the entity
     */
    template <typename T>
    void RemoveComponentFrom(Entity entity, T component)
    {
        GetStorageForComponentType<T>()->RemoveData(entity, component);
    }

    /*
     * Retrieves specified component
     */
    template <typename T>
    T& GetComponent(Entity entity)
    {
        return GetStorageForComponentType<T>()->GetData(entity);
    }

    template <typename T>
    void OnEntityDestroyed(Entity entity)
    {
        for(auto const& pair : m_componentsStorages)
        {
            auto const& component = pair.second;
            component->OnEntityDestroyed(entity);
        }
    }

  private:
    // maps string to the component type
    std::unordered_map<const char*, ECS::ComponentType> m_componentTypes{};
    /* Get the index based on what component we want to find
     * |
     * V
     * Find the component using the retrieved index
     **/
    std::unordered_map<const char*, std::shared_ptr<ECS::IComponentStorage>> m_componentsStorages{};

    ECS::ComponentType m_nextComponentType{};

    template <typename T>
    std::shared_ptr<ComponentStorage<T>> GetStorageForComponentType()
    {
        const char* typeName = typeid(T).name();

        assert(m_componentsStorages.contains(typeName) && "Component not registered did you forget to call ::Register(component) method ? ");

        return std::static_pointer_cast<ComponentStorage<T>>(m_componentsStorages[typeName]);
    }
};

}  // namespace ECS

#endif  //VULKAN_RTX_COMPONENTMANAGER_HPP
