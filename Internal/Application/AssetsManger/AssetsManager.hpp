//
// Created by wpsimon09 on 21/10/24.
//

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

namespace ApplicationCore
{
    class GLTFLoader;
}

struct MaterialPaths;

namespace ApplicationCore
{
    class Mesh;
    class Material;
}

namespace ApplicationCore
{
    class Scene;
}

namespace ApplicationCore
{
    class VertexArray;
}

namespace VulkanCore
{
    class VBufferAllocator;
    class VBuffer;
    class VDevice;
}

namespace std
{
    template<> struct hash<MaterialPaths>
    {
        size_t operator()(const MaterialPaths& other) const
        {
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
    explicit AssetsManager(const VulkanCore::VDevice& device, VulkanCore::VBufferAllocator& bufferAllocator);
    void DeleteAll();
    std::shared_ptr<VertexArray> GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType);

    void GetTexture(std::shared_ptr<VulkanCore::VImage> &texture,const std::string& path);
    void GetTexture(std::shared_ptr<VulkanCore::VImage> &texture,const std::string& textureID,TextureBufferInfo& data);

    void AddMesh(std::string meshName, std::shared_ptr<Mesh> mesh);
    void GetDummyTexture(std::shared_ptr<VulkanCore::VImage> &texture) const {texture = m_dummyTexture;}
    std::shared_ptr<Material> GetDummyMaterial() { return m_dummyMaterial; };

    std::shared_ptr<ApplicationCore::Material> GetMaterial(MaterialPaths& path);
    std::vector<std::shared_ptr<VertexArray>>& GetVertexData()  {return m_vertexArrays;};

    VulkanCore::VBufferAllocator& GetBufferAllocator() {return m_bufferAllocator;}

    bool Sync();

    ~AssetsManager() = default;
private:

    void StartLoadingTexture(std::shared_ptr<VulkanCore::VImage> &texturePtr, const std::string& path);
    void StartLoadingTexture(std::shared_ptr<VulkanCore::VImage>& texture,  const std::string& textureID, TextureBufferInfo& data);

    const VulkanCore::VDevice& m_device;
    VulkanCore::VBufferAllocator& m_bufferAllocator;

    std::vector<std::shared_ptr<VertexArray>> m_vertexArrays;

    std::unordered_map<MaterialPaths, std::shared_ptr<ApplicationCore::Material>> m_materials;

    std::unordered_map<MESH_GEOMETRY_TYPE, std::shared_ptr<VertexArray>> m_preloadedMeshData;

    std::unordered_map<std::string, std::shared_ptr<VulkanCore::VImage>> m_textures; //access only from main thread
    std::unordered_map<std::string, std::future<VulkanStructs::ImageData>> m_texturesToLoad; // accessed only from loading thread
    std::unordered_map<std::string, std::shared_ptr<ApplicationCore::Mesh>> m_meshes;

    std::mutex m_mutex;

    std::shared_ptr<VulkanCore::VImage> m_dummyTexture;
    std::shared_ptr<ApplicationCore::Material> m_dummyMaterial;

    friend class ApplicationCore::GLTFLoader;
};

} // ApplicationCore

#endif //ASSETSMANAGER_HPP
