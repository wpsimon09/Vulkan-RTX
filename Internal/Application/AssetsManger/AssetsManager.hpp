//
// Created by wpsimon09 on 21/10/24.
//

#ifndef ASSETSMANAGER_HPP
#define ASSETSMANAGER_HPP
#include <map>
#include <memory>

#include "Application/Enums/ClientEnums.hpp"

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
    const VertexArray& GetVertexArrayForGeometryType(MESH_GEOMETRY_TYPE geometryType) ;
    ~AssetsManager() = default;

private:
    const VulkanCore::VDevice& m_device;
    std::map<MESH_GEOMETRY_TYPE, std::unique_ptr<VertexArray>> m_meshData;

private:
    void LoadPredefinedMeshes();
};

} // ApplicationCore

#endif //ASSETSMANAGER_HPP
