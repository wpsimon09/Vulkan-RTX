//
// Created by wpsimon09 on 10/12/24.
//

#ifndef GLTFLOADER_HPP
#define GLTFLOADER_HPP
#include <memory>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "fastgltf/base64.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"



namespace ApplicationCore {

struct MaterialPaths;
class VertexArray;
class AssetsManager;
class Mesh;

class SceneNode;

class GLTFLoader {
public:
    GLTFLoader(const ApplicationCore::AssetsManager& assetsManager);

    std::shared_ptr<SceneNode> LoadGLTFScene(std::filesystem::path gltfPath);

    ~GLTFLoader() = default;

private:
    const VulkanCore::VDevice& m_device;
    const ApplicationCore::AssetsManager& m_assetsManager;

    std::shared_ptr<SceneNode> m_rootNode;
    std::vector<std::shared_ptr<SceneNode>> m_topNodes;

    std::shared_ptr<SceneNode> m_modelRoot;
    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::Mesh>> m_meshes;

};

} // ApplicationCore

#endif //GLTFLOADER_HPP
