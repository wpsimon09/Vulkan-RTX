//
// Created by wpsimon09 on 10/12/24.
//

#ifndef GLTFLOADER_HPP
#define GLTFLOADER_HPP
#include "Application/AssetsSystem/VTexture.hpp"


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
class Project;
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
    explicit GLTFLoader(Project& project, ApplicationCore::AssetsManager& assetsManager);

    void LoadGLTFScene(Scene& scene, std::filesystem::path& saveToPath, std::filesystem::path gltfPath, const ImportOptions& importOptions) const;


    ~GLTFLoader() = default;

  private:
    const VulkanCore::VDevice&      m_device;
    ApplicationCore::AssetsManager& m_assetsManager;
    Project&                        m_project;

  private:
    void PostLoadClear();
    void LoadImage(fastgltf::Asset&       asset,
                   std::string            parentPath,
                   fastgltf::Image&       image,
                   std::vector<VTexture>& imageStorage,
                   std::filesystem::path& saveToDirectory) const;
};

}  // namespace ApplicationCore

#endif  //GLTFLOADER_HPP
