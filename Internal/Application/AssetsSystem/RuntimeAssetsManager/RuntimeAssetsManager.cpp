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
}  // namespace ApplicationCore