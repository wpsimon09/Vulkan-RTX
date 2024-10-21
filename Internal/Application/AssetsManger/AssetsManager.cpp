//
// Created by wpsimon09 on 21/10/24.
//

#include "AssetsManager.hpp"

namespace ApplicationCore {
    AssetsManager::AssetsManager(const VulkanCore::VDevice &device):m_device(device) {
        LoadPredefinedMeshes();
    }
} // ApplicationCore