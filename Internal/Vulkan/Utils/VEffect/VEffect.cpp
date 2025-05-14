//
// Created by wpsimon09 on 23/04/25.
//

#include "VEffect.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VResrouceGroup/VResrouceGroup.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace VulkanUtils {
VEffect::VEffect(const VulkanCore::VDevice&                          device,
                 const std::string&                                  name,
                 VulkanCore::VDescriptorLayoutCache&                 descriptoSetLayoutCache,
                 std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet)
    : m_device(device)
    , m_name(name)
    , m_resourceGroup(descriptorSet)
    , m_descriptorSetLayoutCache(descriptoSetLayoutCache)
{
    m_ID = EffectIndexCounter++;
}
std::string& VEffect::GetName()
{
    return m_name;
}
DescriptorSetTemplateVariant& VEffect::GetResrouceGroupStructVariant()
{
    return m_resourceGroup->GetResourceGroupStruct();
}
vk::DescriptorUpdateTemplate& VEffect::GetUpdateTemplate()
{
    return m_resourceGroup->GetUpdateTemplate();
}
unsigned short VEffect::EvaluateRenderingOrder()
{
    return 0;
}
int& VEffect::GetID()
{
    return m_ID;
}
EDescriptorLayoutStruct VEffect::GetLayoutStructType()
{
    return m_resourceGroup->GetResourceGroupStrucutureType();
}
void VEffect::CreateLayouts(const VulkanCore::ReflectionData& reflectionData)
{
    m_reflectionData = &reflectionData;

    m_descriptorSets.reserve(reflectionData.descriptorSets.size());

    for(auto& set : reflectionData.descriptorSets)
    {
        VulkanStructs::VDescriptorSet newSet;

        newSet.layout = m_descriptorSetLayoutCache.CreateDescriptorSetLayout(&set.second.createInfo);

        // copy the layout for pieline creation
        m_descriptorSetLayouts.push_back(newSet.layout);
        newSet.sets.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_device.GetDescriptorAllocator().Allocate(&newSet.sets[i], newSet.layout);


            // create writes
            for(auto& binding : set.second.bindings)
            {
                vk::WriteDescriptorSet write;
                write.descriptorCount = binding.descriptorCount;
                write.descriptorType  = binding.descriptorType;
                write.dstSet          = newSet.sets[i];  // 1 set for the frame
                write.dstBinding      = binding.binding;

                newSet.writes[i][binding.binding] = write;
            }
        }
        m_descriptorSets.push_back(newSet);
    }

    Utils::Logger::LogSuccess("Descriptor set layout created successfully");
}
void VEffect::WriteBuffer(uint32_t frame, uint32_t set, uint32_t binding, vk::DescriptorBufferInfo bufferInfo)
{
    assert(m_descriptorSets[set].writes[frame].contains(binding) && "there is no such binding in the given descirptor set");

    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(write.descriptorType == vk::DescriptorType::eUniformBuffer || write.descriptorType == vk::DescriptorType::eUniformBuffer);

    m_bufferInfos.push_back(bufferInfo);
    write.pBufferInfo = &m_bufferInfos.back();
}

void VEffect::WriteImage(uint32_t frame, uint32_t set, uint32_t binding, vk::DescriptorImageInfo imageInfo)
{
    assert(m_descriptorSets[set].writes[frame].contains(binding) && "there is no such binding in the given descirptor set");
    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(write.descriptorType == vk::DescriptorType::eCombinedImageSampler || write.descriptorType == vk::DescriptorType::eSampledImage || write.descriptorType == vk::DescriptorType::eStorageImage);

    m_imageInfos.push_back(imageInfo);
    write.pImageInfo = &m_imageInfos.back();
}

void VEffect::WriteAccelerationStrucutre(uint32_t frame, uint32_t set, uint32_t binding, vk::AccelerationStructureKHR& asInfo)
{
    assert(m_descriptorSets[set].writes[frame].contains(binding) && "there is no such binding in the given descirptor set");


    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(m_descriptorSets[set].writes[frame][binding].descriptorType == vk::DescriptorType::eAccelerationStructureKHR);

    vk::WriteDescriptorSetAccelerationStructureKHR asWrite;
    asWrite.accelerationStructureCount = 1;
    asWrite.pAccelerationStructures    = &asInfo;
    m_asInfos.push_back(asWrite);

    write.pNext = &m_asInfos.back();
}


void VEffect::ApplyWrites(uint32_t frame)
{
    std::vector<vk::WriteDescriptorSet> writes;

    for(auto& set : m_descriptorSets)
    {
        for(auto& write : set.writes[frame])
        {
            writes.push_back(write.second);
        }
        m_device.GetDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
    }

    m_bufferInfos.clear();
    m_bufferInfos.shrink_to_fit();

    m_imageInfos.clear();
    m_imageInfos.shrink_to_fit();

    m_asInfos.clear();
    m_asInfos.shrink_to_fit();

}



}  // namespace VulkanUtils


// namespace VulkanUtils