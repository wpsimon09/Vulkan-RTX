//
// Created by simon on 23/01/2026.
//

#include "RuntimeAssetsManager.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace ApplicationCore {
RuntimeAssetsManager::RuntimeAssetsManager(Project& project, const VulkanCore::VDevice& device)
    : m_project(project)
    , m_device(device)
    , m_meshDataManager(device.GetMeshDataManager())
{
}
std::shared_ptr<VMesh> RuntimeAssetsManager::LoadMesh(uuid::UUID& uuid) {}

std::shared_ptr<VMaterial> RuntimeAssetsManager::LoadMaterial(uuid::UUID& uuid) {}

std::shared_ptr<VTexture> RuntimeAssetsManager::LoadTexture(uuid::UUID& uuid) {}

void RuntimeAssetsManager::UnloadTexture(std::shared_ptr<VTexture> texture) {}

void RuntimeAssetsManager::UnloadMaterial(std::shared_ptr<VMaterial> material) {}

void RuntimeAssetsManager::UnloadMesh(std::shared_ptr<VMesh> mesh) {}

AssetEntry& RuntimeAssetsManager::GetAssetEntry(uuid::UUID& uuid)
{
    m_project.GetAsset(uuid);
}
}  // namespace ApplicationCore