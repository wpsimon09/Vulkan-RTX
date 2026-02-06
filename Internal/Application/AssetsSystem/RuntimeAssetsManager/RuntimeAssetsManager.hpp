//
// Created by simon on 23/01/2026.
//

#ifndef VULKAN_RTX_RUNTIMEASSETSMANAGER_HPP
#define VULKAN_RTX_RUNTIMEASSETSMANAGER_HPP
#include "Application/AssetsSystem/VAsset2.hpp"
#include "Application/AssetsSystem/VMaterial.hpp"
#include "Application/Utils/Uuid.hpp"


#include <unordered_map>

namespace VulkanCore {
class MeshDatatManager;
}
namespace ApplicationCore {
class VTexture;
class VMesh;
class Project;

class RuntimeAssetsManager
{
  public:
    RuntimeAssetsManager(Project& project, const VulkanCore::VDevice& device);

    std::shared_ptr<VMesh>     LoadMesh(uuid::UUID& uuid);
    std::shared_ptr<VMesh>     LoadMesh(std::filesystem::path& path);
    std::shared_ptr<VMaterial> LoadMaterial(uuid::UUID& uuid);
    std::shared_ptr<VTexture>  LoadTexture(uuid::UUID& uuid);

    void UnloadTexture(std::shared_ptr<VTexture> texture);
    void UnloadMaterial(std::shared_ptr<VMaterial> material);
    void UnloadMesh(std::shared_ptr<VMesh> mesh);

  private:
    AssetEntry& GetAssetEntry(uuid::UUID& uuid);


    Project&                                                   m_project;
    const VulkanCore::VDevice&                                 m_device;
    VulkanCore::MeshDatatManager&                              m_meshDataManager;
    std::unordered_map<uuid::UUID, std::shared_ptr<VMesh>>     m_loadedMeshes;
    std::unordered_map<uuid::UUID, std::shared_ptr<VTexture>>  m_loadedTextures;
    std::unordered_map<uuid::UUID, std::shared_ptr<VMaterial>> m_loadedMaterials;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_RUNTIMEASSETSMANAGER_HPP
