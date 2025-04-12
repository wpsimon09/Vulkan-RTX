//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhrHelpers.hpp"

#include "VRayTracingBuilderKhr.hpp"

VulkanCore::BLASInput VulkanCore::StaticMeshToBLASInput(std::shared_ptr<ApplicationCore::StaticMesh>& mesh,
                                                        glm::mat4 matrix)
{
    // get the address of vertex buffer and indeex buffers
    vk::DeviceAddress vertexAddrss;
    vk::DeviceAddress indexAddrss;
}
