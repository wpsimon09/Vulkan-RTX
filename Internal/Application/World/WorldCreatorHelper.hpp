//
// Created by simon on 13/01/2026.
//

#ifndef VULKAN_RTX_WORLDCREATORHELPER_HPP
#define VULKAN_RTX_WORLDCREATORHELPER_HPP

namespace ApplicationCore {
class World;

//===========================================
// Geometry entities
void AddStaticMeshEntity(World& world);

//===========================================
// VFX entities
void AddFogEntity(World& world);

//===========================================
// Lightning entities
void AddAtmosphereEntity(World& world);
void AddSunLightEntity(World& world);
void AddPointLightEntity(World& world);
void AddAreaLightEntity(World& world);
void AddEnvironmentLightEntity(World& world);

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_WORLDCREATORHELPER_HPP
