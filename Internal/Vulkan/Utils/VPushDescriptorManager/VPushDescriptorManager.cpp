//
// Created by wpsimon09 on 08/11/24.
//

#include "VPushDescriptorManager.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"

namespace VulkanUtils {
    VPushDescriptorManager::VPushDescriptorManager(const VulkanCore::VDevice &device): m_device(device) {
        m_descriptorSetLayout = VulkanCore::VDescriptorSetLayout::Builder(device)
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex,1)
            .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex,1)
            .Build();
    }

    void VPushDescriptorManager::
    WriteBuffer(uint32_t binding, vk::DescriptorBufferInfo &bufferInfo) {
        assert(m_descriptorSetLayout->m_descriptorSetLayoutBindings.count(binding) == 1);
            vk::WriteDescriptorSet descriptorWrite = {};
            // retrieves the type of the descriptor from the binding
            descriptorWrite.descriptorType = m_descriptorSetLayout->m_descriptorSetLayoutBindings[binding].descriptorType;
            descriptorWrite.dstBinding = binding;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;
            descriptorWrite.descriptorCount = 1;

            m_writeDescriptorSets.push_back(descriptorWrite);
    }
} // VulkanUtils