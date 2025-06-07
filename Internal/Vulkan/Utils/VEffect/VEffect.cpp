//
// Created by wpsimon09 on 23/04/25.
//

#include "VEffect.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace VulkanUtils {
VEffect::VEffect(const VulkanCore::VDevice&          device,
                 const std::string&                  name,
                 VulkanCore::VDescriptorLayoutCache& descriptoSetLayoutCache,
                 EShaderBindingGroup                 bindingGroup)
    : m_device(device)
    , m_name(name)
    , m_bindingGroup(bindingGroup)
    , m_descriptorSetLayoutCache(descriptoSetLayoutCache)
{
    m_ID = EffectIndexCounter++;
}
std::string& VEffect::GetName()
{
    return m_name;
}
EShaderBindingGroup VEffect::GetBindingGroup()
{
    return m_bindingGroup;
}

vk::StridedDeviceAddressRegionKHR VEffect::GetShaderBindingTableEntry(VulkanCore::RTX::ERayTracingStageIndices)
{
    throw std::runtime_error("This effect does not support ray tracing ensure that effect you are using is being used for ray tracing");
}
unsigned short VEffect::EvaluateRenderingOrder()
{
    return 0;
}
int& VEffect::GetID()
{
    return m_ID;
}

void VEffect::CreateLayouts(const VulkanCore::ReflectionData& reflectionData)
{
    m_reflectionData = &reflectionData;

    m_descriptorSets.resize(reflectionData.descriptorSets.size());
    m_descriptorSetLayouts.resize(reflectionData.descriptorSets.size());

    for(auto& set : reflectionData.descriptorSets)
    {
        VulkanStructs::VDescriptorSet reflectedSet;

        reflectedSet.layout = m_descriptorSetLayoutCache.CreateDescriptorSetLayout(&set.second.createInfo);

        // copy the layout for pieline creation
        m_descriptorSetLayouts[set.second.setNumber] = reflectedSet.layout;
        reflectedSet.sets.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_device.GetDescriptorAllocator().Allocate(&reflectedSet.sets[i], reflectedSet.layout);


            // create writes
            for(auto& binding : set.second.bindings)
            {
                vk::WriteDescriptorSet write;
                write.descriptorCount = binding.descriptorCount;
                write.descriptorType  = binding.descriptorType;
                write.dstSet          = reflectedSet.sets[i];  // 1 set for the frame
                write.dstBinding      = binding.binding;

                reflectedSet.writes[i][binding.binding] = write;
            }
        }
        m_descriptorSets[set.second.setNumber] = reflectedSet;
    }

    Utils::Logger::LogSuccess("Descriptor set layout created successfully");
}
const VulkanCore::ReflectionData* VEffect::GetReflectionData()
{
    return m_reflectionData;
}

void VEffect::SetNumWrites(uint32_t buffers, uint32_t images, uint32_t accels)
{
    m_bufferInfos.reserve(buffers);
    m_imageInfos.reserve(images);

    // we have to store both the copy of acceleration strucutre handle and a desc write to it
    m_asInfos.reserve(accels);
    m_asWriteInfos.reserve(accels);
}

void VEffect::WriteBuffer(uint32_t frame, uint32_t set, uint32_t binding, vk::DescriptorBufferInfo bufferInfo)
{
    assert(m_bufferInfos.capacity() > 0 && "Before writing to the vector ensure you call SetNumWrites()");
    if(!RelaxedAssert(m_descriptorSets[set].writes[frame].contains(binding),
    "there is no such binding in the given descirptor set" + std::to_string(set)
                      + " binding: " + std::to_string(binding)))
        return;


    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(write.descriptorType == vk::DescriptorType::eUniformBuffer || write.descriptorType == vk::DescriptorType::eStorageBuffer);

    m_bufferInfos.push_back(bufferInfo);
    write.pBufferInfo = &m_bufferInfos[m_bufferInfos.size() - 1];
}

void VEffect::WriteImage(uint32_t frame, uint32_t set, uint32_t binding, vk::DescriptorImageInfo imageInfo)
{
    assert(m_imageInfos.capacity() > 0 && "Before writing to the vector ensure you call SetNumWrites()");
    if(!RelaxedAssert(m_descriptorSets[set].writes[frame].contains(binding),
                      "there is no such binding in the given descirptor set: " + std::to_string(set)
                          + " binding: " + std::to_string(binding)))
        return;

    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(write.descriptorType == vk::DescriptorType::eCombinedImageSampler || write.descriptorType == vk::DescriptorType::eSampledImage
           || write.descriptorType == vk::DescriptorType::eStorageImage);

    m_imageInfos.push_back(imageInfo);
    write.pImageInfo = &m_imageInfos[m_imageInfos.size() - 1];
}
void VEffect::WriteImageArray(uint32_t frame, uint32_t set, uint32_t binding, const std::vector<vk::DescriptorImageInfo>& imageInfos)
{
    assert(m_imageInfos.capacity() > 0 && "Before writing to the vector ensure you call SetNumWrites()");
    assert(m_imageInfos.capacity() >= imageInfos.capacity()
           && "Image array you are trying to write is too small for this set, adjust SetNumWrites accrodingly");
    assert(m_descriptorSets[set].writes[frame].contains(binding) && "there is no such binding in the given descirptor set");

    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(write.descriptorType == vk::DescriptorType::eCombinedImageSampler || write.descriptorType == vk::DescriptorType::eSampledImage
           || write.descriptorType == vk::DescriptorType::eStorageImage);

    m_imageInfos.insert(m_imageInfos.end(), imageInfos.begin(), imageInfos.end());
    write.pImageInfo      = m_imageInfos.data();
    write.descriptorCount = imageInfos.size();
}

void VEffect::WriteAccelerationStrucutre(uint32_t frame, uint32_t set, uint32_t binding, vk::AccelerationStructureKHR asInfo)
{
    assert(m_asInfos.capacity() > 0 && "Before writing to the vector ensure you call SetNumWrites()");

    if(!RelaxedAssert(m_descriptorSets[set].writes[frame].contains(binding), "there is no such binding in the given descirptor set" + std::to_string(set)
                          + " binding: " + std::to_string(binding)))
        return;

    auto& write = m_descriptorSets[set].writes[frame][binding];
    assert(m_descriptorSets[set].writes[frame][binding].descriptorType == vk::DescriptorType::eAccelerationStructureKHR);

    m_asInfos.push_back(asInfo);

    vk::WriteDescriptorSetAccelerationStructureKHR asWrite;
    asWrite.accelerationStructureCount = 1;
    asWrite.pAccelerationStructures    = &m_asInfos[m_asInfos.size() - 1];

    m_asWriteInfos.push_back(asWrite);

    write.pNext = &m_asWriteInfos[m_asInfos.size() - 1];
}


void VEffect::ApplyWrites(uint32_t frame)
{
    std::vector<vk::WriteDescriptorSet> writes;

    for(auto& set : m_descriptorSets)
    {
        for(auto& write : set.writes[frame])
        {
            if(write.second.pBufferInfo != nullptr || write.second.pImageInfo != nullptr || write.second.pNext != nullptr)
            {
                writes.push_back(write.second);
                write.second.pBufferInfo = nullptr;
                write.second.pImageInfo  = nullptr;
                write.second.pNext       = nullptr;
            }
        }
    }
    m_device.GetDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);

    m_bufferInfos.clear();
    m_bufferInfos.shrink_to_fit();

    m_imageInfos.clear();
    m_imageInfos.shrink_to_fit();

    m_asInfos.clear();
    m_asInfos.shrink_to_fit();

    m_asWriteInfos.clear();
    m_asWriteInfos.shrink_to_fit();
}
void VEffect::CmdPushConstant(const vk::CommandBuffer& commandBuffer, const vk::PushConstantsInfo& info)
{
    if(m_reflectionData->PCs.size() > 0)
    {
        commandBuffer.pushConstants2(info);
    }
}


}  // namespace VulkanUtils


// namespace VulkanUtils