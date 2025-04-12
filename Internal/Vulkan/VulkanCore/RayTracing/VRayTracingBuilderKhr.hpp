//
// Created by wpsimon09 on 12/04/25.
//

#ifndef VRAYTRACINGBUILDERKHR_HPP
#define VRAYTRACINGBUILDERKHR_HPP
#include "vulkan/vulkan.hpp"

namespace ApplicationCore
{
    class Scene;
}

namespace VulkanCore
{
    class VDevice;
}

namespace VulkanCore
{
    // each mesh will be stored in one of those
    struct BLASInput
    {
        std::vector<vk::AccelerationStructureGeometryKHR> asGeometry;
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildOffSetInfo;
    };

    struct BLASEntry
    {
        BLASInput input;

    };

    /**
     * This class will act as a Top level AS according to NVIDIA tutorial, so i might redo it later
     *
     */
    class VRayTracingBuilderKHR
    {
    public:
        VRayTracingBuilderKHR(const VulkanCore::VDevice& device, ApplicationCore::Scene& scene);
        void BuildBLAS();
    private:
        const VulkanCore::VDevice& m_device;
        ApplicationCore::Scene& m_scene;
        std::vector<BLASEntry> m_blasEntries;

    };
} // VulkanCore

#endif //VRAYTRACINGBUILDERKHR_HPP
