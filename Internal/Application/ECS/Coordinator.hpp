//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_COORDINATOR_HPP
#define VULKAN_RTX_COORDINATOR_HPP
#include "Types.hpp"
#include "System/SystemManager.hpp"
#include "World/ComponentManager.hpp"
#include "World/EntityManager.hpp"

#include <memory>

namespace ECS {
class SystemManager;
class EntityManager;

/*
 * Ties together entire ECS
 * this what the world will be using
 */
class Coordinator
{
  public:
    Coordinator();

    ECS::Entity CreateEntity();
    ECS::Entity CreateEntityWithMetadata(std::string name, char* icon, std::string tag);
    void        DestroyEntity(Entity entity);
    int         GetAllAliveEntities();

    //===========================================
    template <typename T>
    void RegisterComponent()
    {
        m_componentManager->RegisterComponent<T>();
    }

    template <typename T>
    void AddComponentTo(Entity entity, T component)
    {
        m_componentManager->AddComponentTo<T>(entity, component);

        auto signature = m_entityManager->GetSignature(entity);
        signature.set(m_componentManager->GetComponentType<T>(), true);
        m_entityManager->SetSignature(entity, signature);

        m_systemManager->OnEntitySignatureChanged(entity, signature);
    }

    template <typename T>
    void RemoveComponentFrom(Entity entity)
    {
        m_componentManager->RemoveComponentFrom<T>(entity);

        auto signature = m_entityManager->GetSignature(entity);
        signature.set(m_componentManager->GetComponentType<T>(), false);
        m_entityManager->SetSignature(entity, signature);

        m_systemManager->OnEntitySignatureChanged(entity, signature);
    }

    template <typename T>
    T& GetComponentFrom(Entity entity)
    {
        return m_componentManager->GetComponent<T>(entity);
    }

    template <typename T>
    ComponentType GetComopnentType()
    {
        return m_componentManager->GetComponentType<T>();
    }

    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        return m_systemManager->RegisterSystem<T>();
    }

    template <typename T>
    void SetSystemSiganture(Signature signature)
    {
        m_systemManager->SetSignature<T>(signature);
    }


  private:
    std::unique_ptr<ECS::EntityManager>    m_entityManager;
    std::unique_ptr<ECS::ComponentManager> m_componentManager;
    std::unique_ptr<ECS::SystemManager>    m_systemManager;
};

}  // namespace ECS

#endif  //VULKAN_RTX_COORDINATOR_HPP
