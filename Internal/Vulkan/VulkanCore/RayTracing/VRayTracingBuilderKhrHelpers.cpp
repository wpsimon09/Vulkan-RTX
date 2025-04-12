//
// Created by wpsimon09 on 12/04/25.
//

#include "VRayTracingBuilderKhrHelpers.hpp"

#include "VRayTracingBuilderKhr.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"

VulkanCore::BLASInput VulkanCore::StaticMeshToBLASInput(std::shared_ptr<ApplicationCore::StaticMesh>& mesh,
                                                        glm::mat4 matrix)
{
    // get the address of vertex buffer and indeex buffers
    vk::DeviceAddress vertexAddress = mesh->GetMeshData()->vertexData.bufferAddress;
    vk::DeviceAddress indexAddress = mesh->GetMeshData()->indexData.bufferAddress;

    uint32_t maxPrimitiveCount = (mesh->GetMeshData()->indexData.size/ sizeof(uint32_t)) / 3;

    vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
    // vertices
    triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
    triangles.vertexData.deviceAddress = vertexAddress;
    triangles.vertexStride = sizeof(ApplicationCore::Vertex);
    triangles.maxVertex = (mesh->GetMeshData()->vertexData.size/ sizeof(ApplicationCore::Vertex)) - 1;

    // indices
    triangles.indexData.deviceAddress = mesh->GetMeshData()->indexData.bufferAddress;
    triangles.indexType = vk::IndexType::eUint32;

    // transform
    triangles.transformData = {};

    //specify that data passed to AS are OPAQUE and TRIANGULATE
    vk::AccelerationStructureGeometryKHR asGeometry;
    asGeometry.geometryType = vk::GeometryTypeKHR::eTriangles;
    asGeometry.flags = vk::GeometryFlagBitsNV::eOpaque;
    asGeometry.geometry.triangles = triangles;

    //create range infos
    vk::AccelerationStructureBuildRangeInfoKHR asBuildOffsetInfo;
    asBuildOffsetInfo.firstVertex = 0;
    asBuildOffsetInfo.primitiveCount = maxPrimitiveCount;
    // needs to be offset to the index buffer htat will fatch from the vertices
    asBuildOffsetInfo.primitiveOffset = mesh->GetMeshData()->indexData.offset;
    asBuildOffsetInfo.transformOffset = 0;

    BLASInput input;
    input.asGeometry.emplace_back(asGeometry);
    input.asBuildOffSetInfo.emplace_back(asBuildOffsetInfo);

    return input;
}

