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


namespace VulkanCore
{
    class VImage;
}

namespace ApplicationCore {
    class Material;

    class VertexArray;
class AssetsManager;
class Mesh;

class SceneNode;

class GLTFLoader {
public:
    explicit GLTFLoader(ApplicationCore::AssetsManager& assetsManager);

    std::vector<std::shared_ptr<SceneNode>> LoadGLTFScene(std::filesystem::path gltfPath) const;

    ~GLTFLoader() = default;

private:
    const VulkanCore::VDevice& m_device;
    ApplicationCore::AssetsManager& m_assetsManager;
private:
    void PostLoadClear();
    void LoadImage(fastgltf::Asset& asset,std::string parentPath, fastgltf::Image& image, std::vector<std::shared_ptr<VulkanCore::VImage>>& imageStorage) const ;
};

} // ApplicationCore

#endif //GLTFLOADER_HPP
