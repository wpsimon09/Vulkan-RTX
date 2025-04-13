//
// Created by wpsimon09 on 13/01/25.
//

#ifndef VFACTORIES_HPP
#define VFACTORIES_HPP
#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore {
struct Vertex;
}

namespace VulkanCore {
class MeshDatatManager;
}
namespace VulkanUtils {
VulkanStructs::MeshData AddNewMeshData(VulkanCore::MeshDatatManager&               meshDataManager,
                                       const std::vector<ApplicationCore::Vertex>& vertices,
                                       const std::vector<uint32_t>&                indices);

}


#endif  //VFACTORIES_HPP
