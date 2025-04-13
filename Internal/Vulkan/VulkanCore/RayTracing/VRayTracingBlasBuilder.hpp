//
// Created by wpsimon09 on 13/04/25.
//

#ifndef VRAYTRACINGBLASBUILDER_HPP
#define VRAYTRACINGBLASBUILDER_HPP
#include <vulkan/vulkan.hpp>

namespace VulkanCore::RTX
{
    struct AccelerationStructBuildData;
}

namespace VulkanCore
{
    class VCommandBuffer;
}

namespace VulkanCore
{
    class VDevice;
}

namespace VulkanCore {
namespace RTX {

class VRayTracingBlasBuilder {
public:
    VRayTracingBlasBuilder(const VulkanCore::VDevice& device);

    bool CmdCreateBlas(const VulkanCore::VCommandBuffer& cmdBuffer,
                       std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                       )
};

} // RTX
} // VulkanCore

#endif //VRAYTRACINGBLASBUILDER_HPP
