//
// Created by wpsimon09 on 10/12/24.
//

#ifndef GLTFLOADER_HPP
#define GLTFLOADER_HPP
#include <memory>
#include <unordered_map>
#include <vector>
#include "Vulkan/VulkanCore/Device/VDevice.hpp"


namespace ApplicationCore {

class AssetsManager;
class Mesh;

class SceneNode;

class GLTFLoader {
public:
    GLTFLoader(const VulkanCore::VDevice& device , const ApplicationCore::AssetsManager& assetsManager);

    std::shared_ptr<SceneNode> Load(std::string path);

    ~GLTFLoader() = default;
private:
    const VulkanCore::VDevice& m_device;
    const ApplicationCore::AssetsManager& m_assetsManager;

    std::shared_ptr<SceneNode> m_modelRoot;
    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::Mesh>> m_meshee;
};

} // ApplicationCore

#endif //GLTFLOADER_HPP
