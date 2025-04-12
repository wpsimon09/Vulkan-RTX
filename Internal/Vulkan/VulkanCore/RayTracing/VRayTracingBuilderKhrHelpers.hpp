//
// Created by wpsimon09 on 12/04/25.
//

#ifndef VRAYTRACINGBUILDERKHRHELPERS_HPP
#define VRAYTRACINGBUILDERKHRHELPERS_HPP
#include <memory>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>

namespace ApplicationCore
{
    class StaticMesh;
}

namespace VulkanCore
{
    struct BLASInput;
}

namespace VulkanCore{
    /**
     * Fills in the VkAccelerationStructureGeometryTrianglesDataKHR together with VkAccelerationStructureGeometryKHR VkAccelerationStructureBuildRangeInfoKHR
     * @param mesh mesh to convert
     * @param matrix model matrix of the bottom level AS
     * @return build BLAS input for the given mesh
     */
    VulkanCore::BLASInput StaticMeshToBLASInput(std::shared_ptr<ApplicationCore::StaticMesh>& mesh, glm::mat4 matrix = glm::mat4(1.0f));
}

#endif //VRAYTRACINGBUILDERKHRHELPERS_HPP
