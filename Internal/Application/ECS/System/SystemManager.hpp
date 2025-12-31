//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_SYSTEMMANAGER_HPP
#define VULKAN_RTX_SYSTEMMANAGER_HPP
#include "System.hpp"
#include "Application/ECS/Types.hpp"


#include <memory>
#include <unordered_map>

namespace ECS {

/*
 * Each system will be registered here together with the components it operates on
 * when components of the entity change (are added or removed) we have to account for that
 * by changing the list of the entities in the given system
 * so the hierarchy si like this:
 * entities -> system <- components
 */
class SystemManager
{
  public:
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const char* typeName = typeid(T).name();
        auto        system   = std::make_shared<T>();
        m_systems.insert({typeName, system});
        return system;
    }

    template <typename T>
    void SetSignature(Signature signature)
    {
        const char* typeName = typeid(T).name();
        assert(!m_systems.contains(typeName) && "System is not yet registered");
        m_signatures.insert({typeName, signature});
    }

    void OnEntitySignatureChanged(Entity entity, Signature signature);

    void OnEntityDestroyed(Entity entity);


  private:
    // maps signatures to the system (a.k.a) what components does this system operate on
    std::unordered_map<const char*, ECS::Signature>                m_signatures{};
    std::unordered_map<const char*, std::shared_ptr<ECS::ISystem>> m_systems{};
};

}  // namespace ECS

#endif  //VULKAN_RTX_SYSTEMMANAGER_HPP
