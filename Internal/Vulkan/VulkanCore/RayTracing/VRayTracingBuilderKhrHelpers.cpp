//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhrHelpers.hpp"

#include "VRayTracingBuilderKhr.hpp"
#include "VRayTracingStructs.hpp"
#include "simdjson.h"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/VertexArray/VertexArray.hpp"


VulkanCore::RTX::BLASInput VulkanCore::RTX::StaticMeshToBLASInput(uint32_t meshIndex,std::shared_ptr<ApplicationCore::StaticMesh>& mesh, glm::mat4 matrix)
{
    // get the address of vertex buffer and indeex buffers
    vk::DeviceAddress vertexAddress = mesh->GetMeshData()->vertexData.bufferAddress;
    vk::DeviceAddress indexAddress  = mesh->GetMeshData()->indexData.bufferAddress;

    uint32_t maxPrimitiveCount = (mesh->GetMeshData()->indexData.size / sizeof(uint32_t)) / 3;

    vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
    // vertices
    triangles.vertexFormat             = vk::Format::eR32G32B32Sfloat;
    triangles.vertexData.deviceAddress = vertexAddress + mesh->GetMeshData()->vertexData.offset;
    triangles.vertexStride             = sizeof(ApplicationCore::Vertex);
    triangles.maxVertex                = (mesh->GetMeshData()->vertexData.size / sizeof(ApplicationCore::Vertex)) - 1;

    // indices
    triangles.indexData.deviceAddress = indexAddress  + mesh->GetMeshData()->indexData.offset;
    triangles.indexType               = vk::IndexType::eUint32;

    // transform
    triangles.transformData = {};

    //specify that data passed to AS are OPAQUE and TRIANGULATE
    vk::AccelerationStructureGeometryKHR asGeometry;
    asGeometry.geometryType       = vk::GeometryTypeKHR::eTriangles;
    asGeometry.flags              = vk::GeometryFlagBitsKHR::eOpaque;
    asGeometry.geometry.triangles = triangles;

    //create range infos
    vk::AccelerationStructureBuildRangeInfoKHR asBuildOffsetInfo;
    asBuildOffsetInfo.firstVertex    = 0;
    asBuildOffsetInfo.primitiveCount = maxPrimitiveCount;
    // needs to be offset to the index buffer htat will fatch from the vertices
    asBuildOffsetInfo.primitiveOffset = 0;
    asBuildOffsetInfo.transformOffset = 0;

    BLASInput input;
    input.asGeometry.emplace_back(asGeometry);
    input.asBuildOffSetInfo.emplace_back(asBuildOffsetInfo);
    input.transform = matrix;;
    input.meshIndex = meshIndex;

    return input;
}

VulkanCore::RTX::AccelKHR VulkanCore::RTX::AllocateAccelerationStructure(const VulkanCore::VDevice& device,
                                                                         vk::AccelerationStructureCreateInfoKHR& createInfo)
{
    AccelKHR result;
    result.buffer = std::make_unique<VulkanCore::VBuffer>(device, "BOTTOM LEVEL ACCELERATION STRUCTURE");
    result.buffer->CreateBuffer(createInfo.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
                                                     | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    // create the accelration structure in the device
    vk::AccelerationStructureCreateInfoKHR accelCI = createInfo;
    accelCI.buffer                                 = result.buffer->GetBuffer();

    result.as = device.GetDevice().createAccelerationStructureKHR(accelCI, nullptr, device.DispatchLoader);

    assert((void*)result.as != nullptr && "Failed to create acceleration structure");

    // get its adress
    vk::AccelerationStructureDeviceAddressInfoKHR addressInfo = {};
    addressInfo.accelerationStructure                         = result.as;
    result.address = device.GetDevice().getAccelerationStructureAddressKHR(addressInfo, device.DispatchLoader);

    return result;
}
vk::TransformMatrixKHR VulkanCore::RTX::GlmToMatrix4KHR(glm::mat4& m)
{
  glm::mat4            temp = glm::transpose(m);
  VkTransformMatrixKHR out_matrix;
  memcpy(&out_matrix, &temp, sizeof(VkTransformMatrixKHR));
  return out_matrix;
}
