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
    class Material;

    class VertexArray;
class AssetsManager;
class Mesh;

class SceneNode;

class GLTFLoader {
public:
    GLTFLoader(ApplicationCore::AssetsManager& assetsManager);

    std::shared_ptr<SceneNode> LoadGLTFScene(std::filesystem::path gltfPath);

    ~GLTFLoader() = default;

private:

    const VulkanCore::VDevice& m_device;
    ApplicationCore::AssetsManager& m_assetsManager;

    std::shared_ptr<SceneNode> m_rootNode;
    std::vector<std::shared_ptr<SceneNode>> m_topNodes;
    std::vector<std::shared_ptr<SceneNode>> m_nodes;

    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<std::shared_ptr<VertexArray>> vertexArrays;
    std::vector<std::shared_ptr<VulkanCore::VImage>> m_textures;
    std::vector<std::shared_ptr<Material>> materials;
private:
    void PostLoadClear();
    void LoadImage(fastgltf::Asset& asset,std::string parentPath, fastgltf::Image& image);
};

} // ApplicationCore

#endif //GLTFLOADER_HPP
