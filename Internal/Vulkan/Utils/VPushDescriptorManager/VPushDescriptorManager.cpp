//
// Created by wpsimon09 on 08/11/24.
//

#include "VPushDescriptorManager.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace VulkanUtils {
    VPushDescriptorManager::VPushDescriptorManager(const VulkanCore::VDevice &device): m_device(device) {
        m_descriptorSetLayout = VulkanCore::VDescriptorSetLayout::Builder(device)
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex,1)
            .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex,1)
            .Build();
    }

    void VPushDescriptorManager::
    AddBufferEntry(uint32_t binding, vk::DescriptorBufferInfo &bufferInfo) {
        assert(m_descriptorSetLayout->m_descriptorSetLayoutBindings.count(binding) == 1);
        vk::DescriptorUpdateTemplateEntry entry{};
        entry.descriptorCount = 0;
        entry.offset = 0;
        entry.stride = 0;
        entry.dstBinding = binding;
        entry.descriptorType = m_descriptorSetLayout->m_descriptorSetLayoutBindings[binding].descriptorType;
        entry.dstArrayElement = 0;

        m_descriptorTemplateEntries.push_back(entry);
    }

    void VPushDescriptorManager::CreateUpdateTemplate(const VulkanCore::VGraphicsPipeline &pipeline) {
        vk::DescriptorUpdateTemplateCreateInfo createInfo{};
        createInfo.descriptorUpdateEntryCount = static_cast<uint32_t>(m_descriptorTemplateEntries.size());
        createInfo.pDescriptorUpdateEntries = m_descriptorTemplateEntries.data();
        createInfo.templateType =  vk::DescriptorUpdateTemplateType::ePushDescriptorsKHR;
        createInfo.descriptorSetLayout =nullptr;
        createInfo.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        createInfo.pipelineLayout = pipeline.GetPipelineLayout();

        m_descriptorUpdateTemplate = m_device.GetDevice().createDescriptorUpdateTemplate(createInfo);
    }

} // VulkanUtils