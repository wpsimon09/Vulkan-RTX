//
// Created by wpsimon09 on 10/05/25.
//

#include "VDescriptorAllocator.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VDescriptorAllocator::VDescriptorAllocator(const VDevice& device)
    : VObject()
    , m_device(device)
{
}
void VDescriptorAllocator::Destroy()
{
    for (auto& p: m_freePools) {
        m_device.GetDevice().destroyDescriptorPool(p);
    }
    for (auto& p: m_usedPools) {
        m_device.GetDevice().destroyDescriptorPool(p);
    }
}


} // VulkanCore