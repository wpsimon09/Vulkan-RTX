//
// Created by wpsimon09 on 13/04/25.
//

#ifndef VRAYTRACINGSTRUCTS_HPP
#define VRAYTRACINGSTRUCTS_HPP
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <Vulkan/VulkanCore/Buffer/VBuffer.hpp>


namespace VulkanCore {
class VDevice;
}

namespace VulkanCore::RTX {
struct BLASInput
{
    std::vector<vk::AccelerationStructureGeometryKHR>       asGeometry;
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildOffSetInfo;
    vk::BuildAccelerationStructureFlagsKHR                  flags{0};
    glm::mat4 transform;
    uint32_t meshIndex ;
};

struct BLASEntry
{
    BLASInput input;
};

struct Instance {
    uint32_t blasIndex;
    uint32_t instanceCustomID;
    uint32_t hitGroupID {0};
    uint32_t maks{0xFF};
    glm::mat4 transforms;
};

struct AccelKHR
{
    vk::AccelerationStructureKHR         as      = nullptr;
    std::shared_ptr<VulkanCore::VBuffer> buffer  = nullptr;
    vk::DeviceAddress                    address = {0};
    void                                  Destroy(const VulkanCore::VDevice& device);

};

struct AccelerationStructBuildData
{
    // what type of AS are we building
    std::optional<vk::AccelerationStructureTypeKHR> asType;
    // build range so that we know where in vertex and index buffer the data
    // for AS are
    std::vector<vk::AccelerationStructureGeometryKHR> asGeometry;
    //
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildRangeInfo;
    vk::AccelerationStructureBuildGeometryInfoKHR           asBuildGoemetryInfo;
    vk::AccelerationStructureBuildSizesInfoKHR              asBuildSizesInfo;

    void AddGeometry(const vk::AccelerationStructureGeometryKHR& g, const vk::AccelerationStructureBuildRangeInfoKHR& offset);
    void AddGeometry(const BLASInput& input, uint32_t index = 0);

    bool hasCompactFlag() const
    {
        return static_cast<bool>(asBuildGoemetryInfo.flags & vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);
    }


    // returns size required to build on BLAS
    vk::AccelerationStructureBuildSizesInfoKHR FinalizeGeometry(const VulkanCore::VDevice&             device,
                                                                vk::BuildAccelerationStructureFlagsKHR flags);
    vk::AccelerationStructureCreateInfoKHR     DescribeCreateInfo();
};

struct ScratchSizeInfo
{
    vk::DeviceSize maxScratch   = 0;
    vk::DeviceSize totalScratch = 0;
};
}  // namespace VulkanCore::RTX

#endif  // VRAYTRACINGSTRUCTS_HPP
