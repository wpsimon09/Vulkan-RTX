//
// Created by wpsimon09 on 10/05/25.
//

#include "VDescriptorAllocator.hpp"

#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
VDescriptorAllocator::VDescriptorAllocator(const VDevice& device)
    : VObject()
    , m_device(device)
{
}
void VDescriptorAllocator::Destroy()
{
    for(auto& p : m_freePools)
    {
        m_device.GetDevice().destroyDescriptorPool(p);
    }
    for(auto& p : m_usedPools)
    {
        m_device.GetDevice().destroyDescriptorPool(p);
    }
}
void VDescriptorAllocator::ResetPools() {
    for(auto& p : m_usedPools) {
        m_device.GetDevice().resetDescriptorPool(p);
    }
}


vk::DescriptorPool VDescriptorAllocator::GrabPool()
{
    if(!m_freePools.empty())
    {
        vk::DescriptorPool grabPool = m_freePools.back();
        m_freePools.pop_back();
        return grabPool;
    }
    else
    {
        return VulkanUtils::CreatePool(m_device, m_descriptorSizes, 1000, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    }
}

bool VDescriptorAllocator::Allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout dLayout)
{
    if(m_currentPool == nullptr)
    {
        m_currentPool = GrabPool();
        m_usedPools.push_back(m_currentPool);
    }

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pSetLayouts                   = &dLayout;
    allocInfo.descriptorPool                = m_currentPool;
    allocInfo.descriptorSetCount            = 1;

    bool needsToReAllocate = false;

    switch(m_device.GetDevice().allocateDescriptorSets(&allocInfo, set))
    {
        case vk::Result::eSuccess: {
            return true;
        };
        case vk::Result::eErrorFragmentedPool: {};
        case vk::Result::eErrorOutOfPoolMemory: { needsToReAllocate = true; break;} // create new pool and allocate descriptor set from there
        default: return false; // unknown error
    }

    if (needsToReAllocate) {
        m_currentPool = GrabPool(); // creates new pool
        m_usedPools.push_back(m_currentPool);

        VulkanUtils::Check(m_device.GetDevice().allocateDescriptorSets(&allocInfo, set));
        return true;
    }return false;
}

} // VulkanCore