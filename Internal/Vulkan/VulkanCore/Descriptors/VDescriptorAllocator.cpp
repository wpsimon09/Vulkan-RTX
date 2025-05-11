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
void VDescriptorAllocator::ResetPools()
{
    for(auto& p : m_usedPools)
    {
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
        case vk::Result::eErrorFragmentedPool: {
        };
        case vk::Result::eErrorOutOfPoolMemory: {
            needsToReAllocate = true;
            break;
        }  // create new pool and allocate descriptor set from there
        default:
            return false;  // unknown error
    }

    if(needsToReAllocate)
    {
        m_currentPool = GrabPool();  // creates new pool
        m_usedPools.push_back(m_currentPool);

        VulkanUtils::Check(m_device.GetDevice().allocateDescriptorSets(&allocInfo, set));
        return true;
    }
    return false;
}

//======================================================================================================
//======================================================================================================
// DESCRIPTOR LAYOUT CACHE ALLOCATION
//======================================================================================================
//======================================================================================================

vk::DescriptorSetLayout VDescriptorLayoutCache::CreateDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo* info)
{
    DescriptorSetLayoutInfo layoutInfo;
    layoutInfo.bindings.reserve(info->bindingCount);

    bool isSorted    = true;
    int  lastBinding = -1;


    // copy bindings from the bindings to classes member variable
    for(int i = 0; i < info->bindingCount; i++)
    {
        layoutInfo.bindings.push_back(info->pBindings[i]);
        if(info->pBindings[i].binding > lastBinding)
        {
            lastBinding = info->pBindings[i].binding;
        }
        else
        {
            isSorted = false;
        }
    }

    // sort the bindings so that they go from 0 to 1
    if(!isSorted)
    {
        std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(),
                  [](vk::DescriptorSetLayoutBinding& a, vk::DescriptorSetLayoutBinding& b) {
                      return a.binding < b.binding;
                  });
    }

    auto it = m_layoutCache.find(layoutInfo);
    if(it != m_layoutCache.end())
    {
        // the layout was found so we dont have to recreate it !
        return (*it).second;
    }
    else
    {
        vk::DescriptorSetLayout layout;
        VulkanUtils::Check(m_device.GetDevice().createDescriptorSetLayout(info, nullptr, &layout));

        m_layoutCache[layoutInfo] = layout;
        return layout;
    }
}

bool VDescriptorLayoutCache::DescriptorSetLayoutInfo::operator==(const DescriptorSetLayoutInfo& other) const
{
    if(other.bindings.size() != bindings.size())
    {
        return false;
    }
    else
    {
        for(int i = 0; i < bindings.size(); i++)
        {
            if(other.bindings[i].binding != bindings[i].binding)
            {
                return false;
            }
            if(other.bindings[i].descriptorType != bindings[i].descriptorType)
            {
                return false;
            }
            if(other.bindings[i].descriptorCount != bindings[i].descriptorCount)
            {
                return false;
            }
            if(other.bindings[i].stageFlags != bindings[i].stageFlags)
            {
                return false;
            }
        }
        return true;
    }
}
size_t VDescriptorLayoutCache::DescriptorSetLayoutInfo::hash() const
{
    using std::hash;
    using std::size_t;

    size_t result = hash<size_t>()(bindings.size());

    for(const vk::DescriptorSetLayoutBinding& b : bindings)
    {
        size_t binding_hash = b.binding | static_cast<uint>(b.descriptorType) << 8 | b.descriptorCount << 16
                              | static_cast<uint>(b.stageFlags) << 24;

        result ^= hash<size_t>()(binding_hash);
    }

    return result;
}


//======================================================================================================
//======================================================================================================
// DESCRIPTOR LAYOUT CACHE ALLOCATION
//======================================================================================================
//======================================================================================================

VDescriptorBuilder VDescriptorBuilder::Begin(VDescriptorLayoutCache* layoutCache, VDescriptorAllocator* allocator) {
    VDescriptorBuilder builder;

    builder.m_descLayoutCache = layoutCache;
    builder.m_descAllocator = allocator;
    return builder;
}


}  // namespace VulkanCore