//
// Created by simon on 13/01/2026.
//

#include "WorldCreatorHelper.hpp"

#include "World.hpp"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/StaticMeshComponent.hpp"
void ApplicationCore::AddStaticMeshEntity(World& world)
{
    ECS::StaticMeshComponent mesh;
    auto newEntity = world.GetECS().CreateEntityWithMetadata("Static mesh entity", ICON_FA_CUBE, "");

    world.GetECS().AddComponentTo<ECS::StaticMeshComponent>(newEntity, mesh);
}

void ApplicationCore::AddFogEntity(World& world)
{
    void ApplicationCore::AddAtmosphereEntity(World & world) {}

    void ApplicationCore::AddSunLightEntity(World & world) {}

    void ApplicationCore::AddPointLightEntity(World & world) {}

    void ApplicationCore::AddAreaLightEntity(World & world) {}

    void ApplicationCore::AddEnvironmentLightEntity(World & world) {}