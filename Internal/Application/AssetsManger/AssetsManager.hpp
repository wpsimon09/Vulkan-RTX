//
// Created by wpsimon09 on 21/10/24.
//

#ifndef ASSETSMANAGER_HPP
#define ASSETSMANAGER_HPP
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>

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

class AssetsManager {
public:
    AssetsManager(const VulkanCore::VDevice& device);
    void DeleteAll();
    VertexArray& GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType);
    std::shared_ptr<VulkanCore::VImage> GetTexture(const std::string& path);

    ~AssetsManager() = default;

private:
    std::shared_ptr<VulkanCore::VImage>LoadTexture(const std::string& path);

    const VulkanCore::VDevice& m_device;
    std::map<MESH_GEOMETRY_TYPE, std::unique_ptr<VertexArray>> m_meshData;
    std::map<std::string, std::shared_ptr<VulkanCore::VImage>> m_textures;

    std::shared_mutex m_mutex;

    std::weak_ptr<VulkanCore::VImage> m_defaultTexture;

};

} // ApplicationCore

#endif //ASSETSMANAGER_HPP
