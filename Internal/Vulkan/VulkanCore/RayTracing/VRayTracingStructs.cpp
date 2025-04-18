//
// Created by wpsimon09 on 13/04/25.
//

#include "VRayTracingStructs.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"



void VulkanCore::RTX::AccelKHR::Destroy(const VulkanCore::VDevice& device)
{
    device.GetDevice().destroyAccelerationStructureKHR(as, nullptr, device.DispatchLoader);
    buffer->Destroy();
}


void VulkanCore::RTX::AccelerationStructBuildData::AddGeometry(const vk::AccelerationStructureGeometryKHR&       g,
                                                               const vk::AccelerationStructureBuildRangeInfoKHR& offset)
{
}

void VulkanCore::RTX::AccelerationStructBuildData::AddGeometry(const BLASInput& input, uint32_t index) {}

vk::AccelerationStructureBuildSizesInfoKHR VulkanCore::RTX::AccelerationStructBuildData::FinalizeGeometry(const VulkanCore::VDevice& device,
                                                                                                          vk::BuildAccelerationStructureFlagsKHR flags)
{
    assert(asGeometry.size() > 0 && "NO GEOMETRY WAS ADDED");
    assert(asType.has_value() && "Acceleration strucutre type was not set !");

    asBuildGoemetryInfo.type                     = asType.value();
    asBuildGoemetryInfo.flags                    = flags;
    asBuildGoemetryInfo.mode                     = vk::BuildAccelerationStructureModeKHR::eBuild;
    asBuildGoemetryInfo.srcAccelerationStructure = nullptr;
    asBuildGoemetryInfo.dstAccelerationStructure = nullptr;
    asBuildGoemetryInfo.geometryCount            = static_cast<uint32_t>(asGeometry.size());
    asBuildGoemetryInfo.pGeometries              = asGeometry.data();
    asBuildGoemetryInfo.ppGeometries             = nullptr;
    asBuildGoemetryInfo.scratchData              = {};

    std::vector<uint32_t> maxPrimitiveCount(asBuildRangeInfo.size());

    for(size_t i = 0; i < asBuildRangeInfo.size(); ++i)
    {
        maxPrimitiveCount[i] = asBuildRangeInfo[i].primitiveCount;
    }

    //asBuildSizesInfo will contain the size that is required to build the AS
        device.GetDevice().getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                             &asBuildGoemetryInfo, maxPrimitiveCount.data(),
                                                             &asBuildSizesInfo, device.DispatchLoader);

    return asBuildSizesInfo;
}
vk::AccelerationStructureCreateInfoKHR VulkanCore::RTX::AccelerationStructBuildData::DescribeCreateInfo() {
  assert(asType.has_value() && "Acceleration structure type was not set !");
  assert(asGeometry.size() > 0 && "NO GEOMETRY WAS ADDED"); // geometry is usually 1 for each mesh
  vk::AccelerationStructureCreateInfoKHR createInfo{};
  createInfo.type = asType.value();
  createInfo.size = asBuildSizesInfo.accelerationStructureSize;

  return createInfo;


}
