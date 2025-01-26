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
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

struct TextureBufferInfo;

//=========================
// Forward Declarations
//=========================
namespace ApplicationCore {
    class GLTFLoader;
    class StaticMesh;
    class Material;
    class Scene;
}

namespace VulkanCore {
    class MeshDatatManager;
    class VBuffer;
    class VDevice;
}

struct MaterialPaths;

namespace std {
    template<> struct hash<MaterialPaths> {
        size_t operator()(const MaterialPaths& other) const {
            size_t h1 = std::hash<std::string>{}(other.DiffuseMapPath);
            size_t h2 = std::hash<std::string>{}(other.ArmMapPath);
            size_t h3 = std::hash<std::string>{}(other.NormalMapPath);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

namespace ApplicationCore {

class AssetsManager {
public:
    //=========================
    // Constructor & Destructor
    //=========================
    explicit AssetsManager(const VulkanCore::VDevice& device, VulkanCore::MeshDatatManager& meshDataManager);

    //=========================
    // Public Methods
    //=========================
    void DeleteAll();
    std::shared_ptr<VulkanStructs::MeshData> MeshDataForGeometryType(MESH_GEOMETRY_TYPE geometryType);

    //=========================
    // Textures
    //=========================
    void GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& path);
    void GetTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID, TextureBufferInfo& data);
    void GetDummyTexture(std::shared_ptr<VulkanCore::VImage>& texture) const { texture = m_dummyTexture; }

    //=========================
    // Meshes
    //=========================
    void AddMesh(std::string meshName, std::shared_ptr<StaticMesh> mesh);

    //=========================
    // Materials
    //=========================
    std::shared_ptr<Material> GetDummyMaterial() { return m_dummyMaterial; }
    std::shared_ptr<ApplicationCore::Material> GetMaterial(MaterialPaths& path);

    //=========================
    // Vertex Data
    //=========================
    std::vector<std::shared_ptr<VulkanStructs::MeshData>>& GetVertexData() { return m_allMeshData; }

    //=========================
    // Buffer Allocator
    //=========================
    VulkanCore::MeshDatatManager& GetBufferAllocator() { return m_meshDataManager; }

    //=========================
    // Synchronization
    //=========================
    bool Sync();

private:
    //=========================
    // Private Methods
    //=========================
    void StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texturePtr, const std::string& path);
    void StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texture, const std::string& textureID, TextureBufferInfo& data);

    //=========================
    // Member Variables
    //=========================
    const VulkanCore::VDevice& m_device;
    VulkanCore::MeshDatatManager& m_meshDataManager;

    std::vector<std::shared_ptr<VulkanStructs::MeshData>> m_allMeshData;
    std::unordered_map<MaterialPaths, std::shared_ptr<ApplicationCore::Material>> m_materials;
    std::unordered_map<MESH_GEOMETRY_TYPE, std::shared_ptr<VulkanStructs::MeshData>> m_preloadedMeshData;

    //=========================
    // Texture Management
    //=========================
    std::unordered_map<std::string, std::shared_ptr<VulkanCore::VImage>> m_textures; // Main thread access only
    std::unordered_map<std::string, std::future<VulkanStructs::ImageData>> m_texturesToLoad; // Loading thread access only

    //=========================
    // Mesh Management
    //=========================
    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::StaticMesh>> m_meshes;

    //=========================
    // Synchronization Objects
    //=========================
    std::mutex m_mutex;

    //=========================
    // Default Assets
    //=========================
    std::shared_ptr<VulkanCore::VImage> m_dummyTexture;
    std::shared_ptr<ApplicationCore::Material> m_dummyMaterial;

    friend class ApplicationCore::GLTFLoader;
};

} // namespace ApplicationCore

#endif // ASSETSMANAGER_HPP
