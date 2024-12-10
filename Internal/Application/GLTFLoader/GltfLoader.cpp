//
// Created by wpsimon09 on 10/12/24.
//

#include "GltfLoader.hpp"
#include <fastgltf/core.hpp>

namespace ApplicationCore {
    GLTFLoader::GLTFLoader(const VulkanCore::VDevice& device, const ApplicationCore::AssetsManager& assetsManager) :m_device(device),  m_assetsManager(assetsManager)
    {
        Utils::Logger::LogSuccess("Crated GLTFLoader !");
    }

    std::shared_ptr<SceneNode> GLTFLoader::Load(std::string path)
    {

    }
} // ApplicationCore