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

struct TextureToLoad
{
    std::shared_ptr<VulkanCore::VImage>& image;
    std::future<std::shared_ptr<VulkanCore::VImage>> futureImage;
};

class AssetsManager {
public:
    AssetsManager(const VulkanCore::VDevice& device);
    void DeleteAll();
    VertexArray& GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType);

    void GetTexture(std::shared_ptr<VulkanCore::VImage> &texture,const std::string& path);

    bool Sync();

    ~AssetsManager() = default;

private:

    void StartLoadingTexture(std::shared_ptr<VulkanCore::VImage> &texturePtr, const std::string& path);

    const VulkanCore::VDevice& m_device;
    std::map<MESH_GEOMETRY_TYPE, std::unique_ptr<VertexArray>> m_meshData;

    std::unordered_map<std::string, std::shared_ptr<VulkanCore::VImage>> m_textures; //access only from main thread
    std::unordered_map<std::string, std::unique_ptr<TextureToLoad>> m_texturesToLoad; // accessed only from loading thread

    std::mutex m_mutex;

    std::shared_ptr<VulkanCore::VImage> m_defaultTexture;

};

} // ApplicationCore

#endif //ASSETSMANAGER_HPP
