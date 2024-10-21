//
// Created by wpsimon09 on 21/10/24.
//

#ifndef ASSETSMANAGER_HPP
#define ASSETSMANAGER_HPP
#include <map>
#include <memory>

#include "Application/Enums/ClientEnums.hpp"

namespace VulkanCore
{
    class VBuffer;
    class VDevice;
}

namespace ApplicationCore {

struct MeshAsset
{
    std::unique_ptr<VulkanCore::VBuffer> vertexBuffer;
};

class AssetsManager {
public:
    AssetsManager(const VulkanCore::VDevice& device);
    void DeleteAll();
    ~AssetsManager();

private:
    const VulkanCore::VDevice& m_device;
    std::map<PRIMITIVE_TOPOLOGY, std::unique_ptr<VulkanCore::VBuffer>> m_meshData;

private:
    void LoadPredefinedMeshes();
};

} // ApplicationCore

#endif //ASSETSMANAGER_HPP
