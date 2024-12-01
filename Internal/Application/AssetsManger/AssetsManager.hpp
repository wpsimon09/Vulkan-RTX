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
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

struct MaterialPaths;

namespace ApplicationCore
{
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
    class VBuffer;
    class VDevice;
}

namespace ApplicationCore {


class AssetsManager {
public:
    explicit AssetsManager(const VulkanCore::VDevice& device);
    void DeleteAll();
    VertexArray& GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType);

    void GetTexture(std::shared_ptr<VulkanCore::VImage> &texture,const std::string& path);
    std::shared_ptr<ApplicationCore::Material> GetMaterial(const MaterialPaths& path);
    void GetDummyTexture(std::shared_ptr<VulkanCore::VImage> &texture) const {texture = m_dummyTexture;};

    bool Sync();

    ~AssetsManager() = default;
private:

    void StartLoadingTexture(std::shared_ptr<VulkanCore::VImage> &texturePtr, const std::string& path);

    const VulkanCore::VDevice& m_device;
    std::map<MESH_GEOMETRY_TYPE, std::unique_ptr<VertexArray>> m_meshData;
;
    std::unordered_map<std::string, std::shared_ptr<VulkanCore::VImage>> m_textures; //access only from main thread
    std::unordered_map<MaterialPaths, std::shared_ptr<ApplicationCore::Material>> m_materials;
    std::unordered_map<std::string, std::future<VulkanStructs::ImageData>> m_texturesToLoad; // accessed only from loading thread

    std::mutex m_mutex;

    std::shared_ptr<VulkanCore::VImage> m_dummyTexture;
};

} // ApplicationCore

#endif //ASSETSMANAGER_HPP
