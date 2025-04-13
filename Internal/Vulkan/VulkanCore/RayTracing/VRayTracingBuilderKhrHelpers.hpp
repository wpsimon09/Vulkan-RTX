//
// Created by wpsimon09 on 12/04/25.
//

#ifndef VRAYTRACINGBUILDERKHRHELPERS_HPP
#define VRAYTRACINGBUILDERKHRHELPERS_HPP
#include <memory>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.hpp>

namespace ApplicationCore {
class StaticMesh;
}

namespace VulkanCore::RTX {
struct BLASInput;
}

namespace VulkanCore::RTX {
/**
     * Fills in the VkAccelerationStructureGeometryTrianglesDataKHR together with VkAccelerationStructureGeometryKHR VkAccelerationStructureBuildRangeInfoKHR
     * @param mesh mesh to convert
     * @param matrix model matrix of the bottom level AS
     * @return build BLAS input for the given mesh
     */
VulkanCore::RTX::BLASInput StaticMeshToBLASInput(std::shared_ptr<ApplicationCore::StaticMesh>& mesh,
                                                 glm::mat4 matrix = glm::mat4(1.0f));

bool hasFlag(VkFlags item, VkFlags flag)
{
  return (item & flag) == flag;
}

}  // namespace VulkanCore::RTX

#endif  //VRAYTRACINGBUILDERKHRHELPERS_HPP
