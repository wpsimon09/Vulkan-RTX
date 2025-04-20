//
// Created by wpsimon09 on 13/04/25.
//

#include "VRayTracingStructs.hpp"

#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"



void VulkanCore::RTX::AccelKHR::Destroy(const VulkanCore::VDevice& device)
{
    if (buffer) {
        device.GetDevice().destroyAccelerationStructureKHR(as, nullptr, device.DispatchLoader);
        buffer->Destroy();
    }
}


void VulkanCore::RTX::AccelerationStructBuildData::AddGeometry(const vk::AccelerationStructureGeometryKHR&       g,
                                                               const vk::AccelerationStructureBuildRangeInfoKHR& offset)
{
    asGeometry.push_back(g);
    asBuildRangeInfo.push_back(offset);
}

void VulkanCore::RTX::AccelerationStructBuildData::AddGeometry(const BLASInput& input, uint32_t index) {}

void VulkanCore::RTX::AccelerationStructBuildData::AddGeometry(const AccelerationStructureGeometryInfo& gemetry)
{
    asGeometry.push_back(gemetry.asGeometry);
    asBuildRangeInfo.push_back(gemetry.buildRangeInfo);
}

VulkanCore::RTX::AccelerationStructureGeometryInfo VulkanCore::RTX::AccelerationStructBuildData::MakeInstanceGeometry(size_t numInstances,
                                                                                                                      vk::DeviceAddress instanceBufferAdress)
{
    assert(asType == vk::AccelerationStructureTypeKHR::eTopLevel && " This method can only be used for TLASes ");

    // instance data buffer is used to inform TLAS what BLASes are in it and their corresponding tranform matrix
    vk::AccelerationStructureGeometryInstancesDataKHR instanceData{};
    instanceData.data.deviceAddress = instanceBufferAdress;

    // set up asGeometry to use the insatnace data specified above
    vk::AccelerationStructureGeometryKHR instanceGeometry{};
    instanceGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
    instanceGeometry.geometry.instances = instanceData;

    // specify the number of primitives, since this is  for TLAS only this will specify the number of BLAS es
    vk::AccelerationStructureBuildRangeInfoKHR buildRanges{};
    buildRanges.primitiveCount = numInstances;

    AccelerationStructureGeometryInfo result;
    result.asGeometry = instanceGeometry; // governs the data for instances (blas index and transform matrix)
    result.buildRangeInfo = buildRanges; // governs the number of instances

    return result;

}

vk::AccelerationStructureBuildSizesInfoKHR VulkanCore::RTX::AccelerationStructBuildData::FinalizeGeometry(const VulkanCore::VDevice& device,
                                                                                                          vk::BuildAccelerationStructureFlagsKHR flags)
{
    assert(asGeometry.size() > 0 && "NO GEOMETRY WAS ADDED");
    assert(asType.has_value() && "Acceleration strucutre type was not set !");

    asBuildInfo.type                     = asType.value();
    asBuildInfo.flags                    = flags;
    asBuildInfo.mode                     = vk::BuildAccelerationStructureModeKHR::eBuild;
    asBuildInfo.srcAccelerationStructure = nullptr;
    asBuildInfo.dstAccelerationStructure = nullptr;
    asBuildInfo.geometryCount            = static_cast<uint32_t>(asGeometry.size());
    asBuildInfo.pGeometries              = asGeometry.data();
    asBuildInfo.ppGeometries             = nullptr;
    asBuildInfo.scratchData              = {};

    std::vector<uint32_t> maxPrimitiveCount(asBuildRangeInfo.size());

    for(size_t i = 0; i < asBuildRangeInfo.size(); ++i)
    {
        maxPrimitiveCount[i] = asBuildRangeInfo[i].primitiveCount;
    }

    //asBuildSizesInfo will contain the size that is required to build the AS
        device.GetDevice().getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                             &asBuildInfo, maxPrimitiveCount.data(),
                                                             &asBuildSizesInfo, device.DispatchLoader);

    return asBuildSizesInfo;
}
vk::AccelerationStructureCreateInfoKHR VulkanCore::RTX::AccelerationStructBuildData::DescribeCreateInfo()
{
    assert(asType.has_value() && "Acceleration structure type was not set !");
    assert(asGeometry.size() > 0 && "NO GEOMETRY WAS ADDED");  // geometry is usually 1 for each mesh
    vk::AccelerationStructureCreateInfoKHR createInfo{};
    createInfo.type = asType.value();
    createInfo.size = asBuildSizesInfo.accelerationStructureSize;

    return createInfo;
}
void VulkanCore::RTX::AccelerationStructBuildData::CmdUpdateAs(const vk::CommandBuffer&                 cmdBuffer,
                                                               vk::AccelerationStructureKHR             as,
                                                               vk::DeviceAddress                        scratchAddress,
                                                               const vk::detail::DispatchLoaderDynamic& dispatchLoader)
{
    assert(asGeometry.size() == asBuildRangeInfo.size() && "number of geometry does not match number of ranges that should be updated ");
    assert(as != VK_NULL_HANDLE
           && "Acceleration structure was not created, make sure your are calling CmdCreateAccelraitonStructure or equivalent");

    const vk::AccelerationStructureBuildRangeInfoKHR* rangeInfo = asBuildRangeInfo.data();

    asBuildInfo.mode                      = vk::BuildAccelerationStructureModeKHR::eUpdate;
    asBuildInfo.srcAccelerationStructure  = as;
    asBuildInfo.dstAccelerationStructure  = as;
    asBuildInfo.scratchData.deviceAddress = scratchAddress;
    asBuildInfo.pGeometries               = asGeometry.data();

    cmdBuffer.buildAccelerationStructuresKHR(1, &asBuildInfo, &rangeInfo, dispatchLoader);

    // since noe scratch buffer is used, place memory barrier to ensure proper synchronization
    VulkanUtils::PlaceAccelerationStructureMemoryBarrier(cmdBuffer, vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                                         vk::AccessFlagBits::eAccelerationStructureReadKHR);
}
void VulkanCore::RTX::AccelerationStructBuildData::CmdBuildAs(const vk::CommandBuffer&                 cmdBuffer,
                                                              vk::AccelerationStructureKHR             as,
                                                              vk::DeviceAddress                        scratchAddress,
                                                              const vk::detail::DispatchLoaderDynamic& dispatch_loader)
{
    assert(asGeometry.size() == asBuildRangeInfo.size() && "number of geometry does not match number of ranges that should be updated ");
    assert(as != VK_NULL_HANDLE
           && "Acceleration structure was not created, make sure your are calling CmdCreateAccelraitonStructure or equivalent");

    const vk::AccelerationStructureBuildRangeInfoKHR* rangeInfo = asBuildRangeInfo.data();

    asBuildInfo.mode                      = vk::BuildAccelerationStructureModeKHR::eBuild;
    asBuildInfo.srcAccelerationStructure  = nullptr;
    asBuildInfo.dstAccelerationStructure  = as;
    asBuildInfo.scratchData.deviceAddress = scratchAddress;
    asBuildInfo.pGeometries               = asGeometry.data();

    cmdBuffer.buildAccelerationStructuresKHR(1, &asBuildInfo, &rangeInfo, dispatch_loader);

    // since noe scratch buffer is used, place memory barrier to ensure proper synchronization
    VulkanUtils::PlaceAccelerationStructureMemoryBarrier(cmdBuffer, vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                                         vk::AccessFlagBits::eAccelerationStructureReadKHR);
}
