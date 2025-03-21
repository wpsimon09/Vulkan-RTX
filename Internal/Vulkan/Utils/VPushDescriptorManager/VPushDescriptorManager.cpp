//
// Created by wpsimon09 on 08/11/24.
//

#include "VPushDescriptorManager.hpp"

#include "VPushDescriptor.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace VulkanUtils {
    VPushDescriptorManager::VPushDescriptorManager(const VulkanCore::VDevice &device): m_device(device) {
        m_descriptorSetLayout = VulkanCore::VDescriptorSetLayout::Builder(device)
            // global datat
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex,1)
            // per object data
            .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex,1)
            // per material datat
            .AddBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment,1) // material features
            .AddBinding(3, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment,1) // material no texture data
            .AddBinding(4, vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,1) // albedo
            .AddBinding(5, vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,1) // normal
            .AddBinding(6, vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,1) // arm
            .AddBinding(7, vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,1) // emissive

            .AddBinding(8, vk::DescriptorType::eUniformBuffer,vk::ShaderStageFlagBits::eFragment,1) //directional light

            .AddBinding(9, vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,1) // ltc
            .AddBinding(10, vk::DescriptorType::eCombinedImageSampler,vk::ShaderStageFlagBits::eFragment,1) // ltc_inverse


            .Build();

            VulkanUtils::UnlitSingleTexture unlitSingleTexture{};
            auto layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, unlitSingleTexture);

            m_pushDescriptors[EDescriptorLayoutStruct::UnlitSingleTexture] =
                std::make_shared<VPushDescriptorSet>(m_device, std::string("Unlit single texture"),std::move(layout));

            VulkanUtils::ForwardShadingDstSet forwardShading{};
            layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, forwardShading);

            m_pushDescriptors[EDescriptorLayoutStruct::ForwardShading] =
                std::make_shared<VPushDescriptorSet>(m_device, std::string("Forward shading dst set"), std::move(layout));

            VulkanUtils::BasicDescriptorSet basicDescriptorSet{};
            layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, basicDescriptorSet);

            m_pushDescriptors[EDescriptorLayoutStruct::Basic] =
                std::make_shared<VPushDescriptorSet>(m_device, std::string("Basic descriptor set"), std::move(layout));


    }

    void VPushDescriptorManager::
    AddUpdateEntry(uint32_t binding, size_t offset, size_t stride) {
        assert(m_descriptorSetLayout->m_descriptorSetLayoutBindings.count(binding) == 1 && "Binding is not part of the descriptor layout !");
        vk::DescriptorUpdateTemplateEntry entry{};
        entry.descriptorCount = 1;
        entry.offset = offset;
        entry.stride = stride;
        entry.dstBinding = binding;
        entry.descriptorType = m_descriptorSetLayout->m_descriptorSetLayoutBindings[binding].descriptorType;
        entry.dstArrayElement = 0;

        m_descriptorTemplateEntries.push_back(entry);

    }


    void VPushDescriptorManager::CreateUpdateTemplate(const VulkanCore::VGraphicsPipeline &pipeline) {
        Utils::Logger::LogInfo("Creating update template object....");
        assert(!m_descriptorTemplateEntries.empty() && "No template entries found");
        vk::DescriptorUpdateTemplateCreateInfo createInfo{};
        createInfo.descriptorUpdateEntryCount = static_cast<uint32_t>(m_descriptorTemplateEntries.size());
        createInfo.pDescriptorUpdateEntries = m_descriptorTemplateEntries.data();
        createInfo.templateType =  vk::DescriptorUpdateTemplateType::ePushDescriptors;
        createInfo.descriptorSetLayout =nullptr;
        createInfo.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        createInfo.pipelineLayout = pipeline.GetPipelineLayout();

        m_descriptorUpdateTemplate = m_device.GetDevice().createDescriptorUpdateTemplate(createInfo);

        assert(m_descriptorUpdateTemplate);
        Utils::Logger::LogSuccess(  "Update template created !");
    }

    void VPushDescriptorManager::Destroy() {
        m_descriptorSetLayout->Destroy();
        m_device.GetDevice().destroyDescriptorUpdateTemplate(m_descriptorUpdateTemplate);
        for (auto& pushDst: m_pushDescriptors)
        {
            pushDst.second->Destroy();
        }
    }

} // VulkanUtils