/*
 * Created by wpsimon09 on 21/10/24.
 */

#ifndef ASSETSMANAGER_HPP
#define ASSETSMANAGER_HPP

#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "Application/Enums/ClientEnums.hpp"
#include "Application/Rendering/Material/MaterialStructs.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Mesh/MeshData.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"


namespace VulkanUtils {
class VEffect;
}
namespace VulkanUtils {
class VEnvLightGenerator;
}

namespace ApplicationCore {
class SkyBoxMaterial;
}

namespace ApplicationCore {
enum class EEffectType : std::uint8_t;
}

namespace ApplicationCore {
class EffectsLibrary;
}

namespace VulkanUtils {
class VRasterEffect;
class VTransferOperationsManager;
}  // namespace VulkanUtils

struct TextureBufferInfo;

//=========================
// Forward Declarations
//=========================
namespace ApplicationCore {
class SceneNode;
class GLTFLoader;
class StaticMesh;
class PBRMaterial;
class Scene;
class VTextureAsset;
}  // namespace ApplicationCore

namespace VulkanCore {
class VImage2;
class MeshDatatManager;
class VBuffer;
class VDevice;
}  // namespace VulkanCore

struct MaterialPaths;

namespace std {
template <>
struct hash<MaterialPaths>
{
    size_t operator()(const MaterialPaths& other) const
    {
        size_t h1 = std::hash<std::string>{}(other.DiffuseMapPath);
        size_t h2 = std::hash<std::string>{}(other.ArmMapPath);
        size_t h3 = std::hash<std::string>{}(other.NormalMapPath);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
}  // namespace std

namespace ApplicationCore {

class AssetsManager
{
  public:
    //=========================
    // Constructor & Destructor
    //=========================
    explicit AssetsManager(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary);

    //=========================
    // Public Methods
    //=========================
    void                        DeleteAll();
    std::shared_ptr<StaticMesh> GetDefaultMesh(EMeshGeometryType geometryType);

    //=========================
    // Transfer ops manager
    //=========================
    VulkanUtils::VTransferOperationsManager& GetTransferOperationsManager() const { return m_transferOpsManager; }

    //=========================
    // Textures
    //=========================
    void GetTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture, const std::string& path, bool saveToDisk = false);
    void GetTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture,
                    const std::string&                               textureID,
                    TextureBufferInfo&                               data,
                    bool                                             saveToDisk = false);
    void GetHDRTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture, const std::string& path, bool saveToDisk = false);
    void GetDummyTexture(std::shared_ptr<ApplicationCore::VTextureAsset>& texture) const { texture = m_dummyTexture; }
    std::shared_ptr<VulkanCore::VImage2> GetDummyTextureImage() const { return m_dummyImage; };
    std::vector<TextureBufferView>       ReadBackAllTextures(std::vector<std::byte>& data);
    //=========================
    // Meshes
    //=========================
    void AddMesh(std::string meshName, std::shared_ptr<StaticMesh> mesh);
    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::StaticMesh>>& GetMeshes() { return m_meshes; }
    VulkanCore::MeshDatatManager&                                                  GetMeshDataManager();

    //=========================
    // Materials
    //=========================
    std::shared_ptr<PBRMaterial>              GetDummyMaterial() { return m_dummyMaterial; }
    std::vector<std::shared_ptr<PBRMaterial>> GetAllMaterials() const;
    void                                      AddMaterial(MaterialPaths& paths, std::shared_ptr<PBRMaterial> material);
    std::shared_ptr<StaticMesh>               GetEditorBilboardMesh(EEditorIcon icon);
    std::shared_ptr<SkyBoxMaterial>           AddSkyBoxMaterial(const std::filesystem::path& path);
    const std::vector<std::shared_ptr<ApplicationCore::SkyBoxMaterial>>& GetAllSkyBoxMaterials() const;


    //=========================
    // Buffer Allocator
    //=========================
    VulkanCore::MeshDatatManager& GetBufferAllocator() { return m_meshDataManager; }

    //=========================
    // Effects
    //=========================
    EffectsLibrary&                                                    GetEffectsLibrary() { return m_effectsLibrary; }
    std::map<EEffectType, std::shared_ptr<VulkanUtils::VEffect>>       GetEffects() const;
    std::map<EEffectType, std::shared_ptr<VulkanUtils::VRasterEffect>> GetAllRasterEffects() const;

    //=========================
    // Buffer Allocator
    //=========================
    void AddModel(std::string path, std::vector<std::shared_ptr<ApplicationCore::SceneNode>>& model);
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> GetModel(const std::string& path);
    std::unordered_map<std::string, std::vector<std::shared_ptr<ApplicationCore::SceneNode>>>& GetModels()
    {
        return m_models;
    }
    void DestroySkyBoxMaterial(const std::string& name);
    ;
    //=========================
    // Synchronization
    //=========================
    bool Sync();

  private:
    //=========================
    // Private Methods
    //=========================
    void CreateDefaultAssets();

    //=========================
    // Member Variables
    //=========================
    const VulkanCore::VDevice&    m_device;
    VulkanCore::MeshDatatManager& m_meshDataManager;

    std::vector<std::shared_ptr<ApplicationCore::PBRMaterial>> m_materials;

    //=========================
    // Texture Management
    //=========================
    std::unordered_map<std::string, std::shared_ptr<VulkanCore::VImage>> m_textures;  // Main thread access only
    std::unordered_map<std::string, std::future<VulkanStructs::VImageData<>>> m_texturesToLoad;  // Loading thread access only

    //=========================
    // Mesh Management
    //=========================
    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::StaticMesh>> m_meshes;

    //=========================
    // Synchronization Objects
    //=========================
    std::mutex m_mutex;

    //========================
    // MODELS
    //========================
    std::unordered_map<std::string, std::vector<std::shared_ptr<ApplicationCore::SceneNode>>> m_models;

    //=========================
    // Default Assets
    //=========================

    std::shared_ptr<ApplicationCore::VTextureAsset>                  m_dummyTexture;
    std::shared_ptr<VulkanCore::VImage2>                             m_dummyImage;
    std::shared_ptr<ApplicationCore::PBRMaterial>                    m_dummyMaterial;
    std::unordered_map<EEditorIcon, std::shared_ptr<PBRMaterial>>    m_editorIconsMaterials;
    std::unordered_map<EMeshGeometryType, VulkanStructs::VMeshData2> m_preloadedMeshes;


    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::VTextureAsset>> m_textures2;

    std::vector<std::shared_ptr<ApplicationCore::VTextureAsset>> m_texturesToSync;

    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::VTextureAsset>> m_HDRTextures;
    std::vector<std::shared_ptr<ApplicationCore::SkyBoxMaterial>>                    m_skyBoxMaterials;

    VulkanUtils::VTransferOperationsManager& m_transferOpsManager;

    ApplicationCore::EffectsLibrary& m_effectsLibrary;

    friend class ApplicationCore::GLTFLoader;
};

}  // namespace ApplicationCore

#endif  // ASSETSMANAGER_HPP
