//
// Created by simon on 31/12/2025.
//

#include "ECSCoordinator.hpp"

#include "Components/MetadataComponent.hpp"
#include "System/SystemManager.hpp"
#include "World/ComponentManager.hpp"
#include "World/EntityManager.hpp"
#include "fastgltf/types.hpp"

namespace ECS {
ECSCoordinator::ECSCoordinator()
{
    m_componentManager = std::make_unique<ComponentManager>();
    m_entityManager    = std::make_unique<EntityManager>();
    m_systemManager    = std::make_unique<SystemManager>();
}

ECS::Entity ECSCoordinator::CreateEntity()
{
    return m_entityManager->CreateEntity();
}
ECS::Entity ECSCoordinator::CreateEntityWithMetadata(const std::string& name, const char* icon, std::string tag)
{
    auto entity   = m_entityManager->CreateEntity();
    auto metaData = ECS::MetadataComponent(name.c_str(), ICON_FA_INFO, 0, "");

    this->AddComponentTo<MetadataComponent>(entity, metaData);

    return entity;
}

void ECSCoordinator::DestroyEntity(Entity entity)
{
    m_componentManager->OnEntityDestroyed(entity);
    m_systemManager->OnEntityDestroyed(entity);
    m_entityManager->DestroyEntity(entity);
}
int ECSCoordinator::GetAllAliveEntities()
{
    return m_entityManager->GetLivingEntityCount();
}
Signature ECSCoordinator::GetSignatureOf(Entity entity)
{
    return m_entityManager->GetSignature(entity);
}
int ECSCoordinator::GetNumberOfRegisteredComponents()
{
    return m_registeredComponents;
}
const std::vector<ECS::Entity>& ECSCoordinator::GetAliveEntities()
{
    return m_entityManager->GetAliveEntities();
}
}  // namespace ECS