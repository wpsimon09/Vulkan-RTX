//
// Created by simon on 13/01/2026.
//

#include "WorldCreatorHelper.hpp"

#include "World.hpp"
#include "Application/ECS/ECSCoordinator.hpp"
#include "Application/ECS/Components/AtmosphereComponent.hpp"
#include "Application/ECS/Components/FogComponent.hpp"
#include "Application/ECS/Components/LightComponents.hpp"
#include "Application/ECS/Components/StaticMeshComponent.hpp"
void ApplicationCore::AddStaticMeshEntity(World& world)
{
    ECS::StaticMeshComponent mesh;
    auto newEntity = world.GetECS().CreateEntityWithMetadata("Static mesh entity", ICON_FA_CUBE, "");
    world.GetECS().AddComponentTo<ECS::StaticMeshComponent>(newEntity, mesh);
}

void ApplicationCore::AddFogEntity(World& world)
{
    ECS::FogComponent fog;
    auto              newEntity = world.GetECS().CreateEntityWithMetadata("Fog entity", ICON_FA_SMOG, "");
    world.GetECS().AddComponentTo<ECS::FogComponent>(newEntity, fog);
}

void ApplicationCore::AddAtmosphereEntity(World& world)
{
    ECS::AtmosphereComponent atmosphere;
    auto                     newEntity = world.GetECS().CreateEntityWithMetadata("Atmosphere entity", ICON_FA_CUBE, "");
    world.GetECS().AddComponentTo<ECS::AtmosphereComponent>(newEntity, atmosphere);
}

void ApplicationCore::AddSunLightEntity(World& world)
{
    ECS::DirectionalLightComponent sunLight;
    auto                           newEntit = world.GetECS().CreateEntityWithMetadata("Sun light", ICON_FA_SUN, "");
    world.GetECS().AddComponentTo<ECS::DirectionalLightComponent>(newEntit, sunLight);
}

void ApplicationCore::AddPointLightEntity(World& world)
{
    ECS::PointLightComponent pointLight;
    auto                     newEntity = world.GetECS().CreateEntityWithMetadata("Point light", ICON_FA_LIGHTBULB, "");
    world.GetECS().AddComponentTo<ECS::PointLightComponent>(newEntity, pointLight);
}

void ApplicationCore::AddAreaLightEntity(World& world)
{
    ECS::AreaLightComponent area;
    auto newEntity = world.GetECS().CreateEntityWithMetadata("Area light", ICON_FA_VECTOR_SQUARE, "");
    world.GetECS().AddComponentTo<ECS::AreaLightComponent>(newEntity, area);
}

void ApplicationCore::AddEnvironmentLightEntity(World& world)
{
    ECS::EnvironemntLightComponent envLight;
    auto newEntity = world.GetECS().CreateEntityWithMetadata("Environment light", ICON_FA_CUBE, "");
    world.GetECS().AddComponentTo<ECS::EnvironemntLightComponent>(newEntity, envLight);
}