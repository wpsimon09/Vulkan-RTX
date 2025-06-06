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


namespace VulkanCore {
class VImage;
}

namespace ApplicationCore {
class Scene;
class VTextureAsset;
class PBRMaterial;

class VertexArray;
class AssetsManager;
class StaticMesh;

class SceneNode;

struct ImportOptions
{
    bool importMaterials     = true;
    bool importOnlyMaterials = false;

    float uniformScale = 1.0f;
};

class GLTFLoader
{
  public:
    explicit GLTFLoader(ApplicationCore::AssetsManager& assetsManager);

    void LoadGLTFScene(Scene& scene, std::filesystem::path gltfPath, const ImportOptions& importOptions) const;


    ~GLTFLoader() = default;

  private:
    const VulkanCore::VDevice&      m_device;
    ApplicationCore::AssetsManager& m_assetsManager;

  private:
    void PostLoadClear();
    void LoadImage(fastgltf::Asset&                                              asset,
                   std::string                                                   parentPath,
                   fastgltf::Image&                                              image,
                   std::vector<std::shared_ptr<ApplicationCore::VTextureAsset>>& imageStorage,
                   bool                                                          saveToDisk = true) const;
};

}  // namespace ApplicationCore

#endif  //GLTFLOADER_HPP
