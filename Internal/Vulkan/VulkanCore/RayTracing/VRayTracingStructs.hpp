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
    glm::mat4                                               transform;
    uint32_t                                                meshIndex;
};

struct BLASEntry
{
    BLASInput input;
};

struct Instance
{
    uint32_t  blasIndex;
    uint32_t  instanceCustomID;
    uint32_t  hitGroupID{0};
    uint32_t  maks{0xFF};
    glm::mat4 transforms;
};

struct AccelKHR
{
    vk::AccelerationStructureKHR         as      = nullptr;
    std::shared_ptr<VulkanCore::VBuffer> buffer  = nullptr;
    vk::DeviceAddress                    address = {0};
    void                                 Destroy(const VulkanCore::VDevice& device);
};

struct AccelerationStructureGeometryInfo
{
    vk::AccelerationStructureGeometryKHR       asGeometry;
    vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo;
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
    vk::AccelerationStructureBuildGeometryInfoKHR           asBuildInfo;
    vk::AccelerationStructureBuildSizesInfoKHR              asBuildSizesInfo;

    void AddGeometry(const vk::AccelerationStructureGeometryKHR& g, const vk::AccelerationStructureBuildRangeInfoKHR& offset);
    void AddGeometry(const BLASInput& input, uint32_t index = 0);
    void AddGeometry(const AccelerationStructureGeometryInfo& gemetry);

    bool hasCompactFlag() const
    {
        return static_cast<bool>(asBuildInfo.flags & vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);
    }

    /**
     * references the buffer that was used to sotre all instances that are withing the TLAS ,this function can only be used
     * this function can only be used if type of build data is for top level acceleration strucutre
    */
    AccelerationStructureGeometryInfo MakeInstanceGeometry(size_t numInstances, vk::DeviceAddress instanceBufferAdress);

    // returns size required to build on BLAS
    vk::AccelerationStructureBuildSizesInfoKHR FinalizeGeometry(const VulkanCore::VDevice&             device,
                                                                vk::BuildAccelerationStructureFlagsKHR flags);
    vk::AccelerationStructureCreateInfoKHR     DescribeCreateInfo();

    void CmdUpdateAs(const vk::CommandBuffer&                 cmdBuffer,
                     vk::AccelerationStructureKHR             as,
                     vk::DeviceAddress                        scratchAddress,
                     const vk::detail::DispatchLoaderDynamic& dispatchLoader);

    void CmdBuildAs(const vk::CommandBuffer&                 cmdBuffer,
                    vk::AccelerationStructureKHR             as,
                    vk::DeviceAddress                        scratchAddress,
                    const vk::detail::DispatchLoaderDynamic& dispatch_loader);
};

struct ScratchSizeInfo
{
    vk::DeviceSize maxScratch   = 0;
    vk::DeviceSize totalScratch = 0;
};


struct RTXShaderPaths {
    std::filesystem::path rayGenPath;
    std::filesystem::path missPath;
    std::filesystem::path rayHitPath;
    /**
     TODO: *Others later
     */
};

enum ERayTracingStageIndices {
    RayGen = 0,
    Miss,
    ClosestHit,
    // TODO: insert new shaders in this exact spot
    ShaderGroupCount, // holds count of all indices
};

}  // namespace VulkanCore::RTX

#endif  // VRAYTRACINGSTRUCTS_HPP
