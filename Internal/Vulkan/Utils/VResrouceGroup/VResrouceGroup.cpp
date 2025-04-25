//
// Created by wpsimon09 on 20/03/25.
//
#include "VResrouceGroup.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

VulkanUtils::VShaderResrouceGroup::VShaderResrouceGroup(const VulkanCore::VDevice&                        device,
                                                        const std::string&                                name,
                                                        std::unique_ptr<VulkanCore::VDescriptorSetLayout> dstLayout)
    : m_device(device)
    , m_name(name)
    , m_dstLayout(std::move(dstLayout))
{
    std::visit(
        [this](auto& templateStruct) {
            using t = std::decay_t<decltype(templateStruct)>;

            //if descriptors change this is where i have to update them
            if constexpr(std::is_same_v<t, VulkanUtils::EmtpyDescriptorSet>)
            {
                m_layoutStructType = EDescriptorLayoutStruct::Empty;
            }

            if constexpr(std::is_same_v<t, VulkanUtils::BasicDescriptorSet>)
            {
                this->AddUpdateEntry(0, offsetof(VulkanUtils::BasicDescriptorSet, buffer1), 0);
                this->AddUpdateEntry(1, offsetof(VulkanUtils::BasicDescriptorSet, buffer2), 0);
                this->AddUpdateEntry(2, offsetof(VulkanUtils::BasicDescriptorSet, buffer3), 0);
                m_layoutStructType = EDescriptorLayoutStruct::Basic;
            }
            else if constexpr(std::is_same_v<t, VulkanUtils::UnlitSingleTexture>)
            {
                // Handle UnlitSingleTexture
                // Example: Update descriptor set entries for UnlitSingleTexture
                this->AddUpdateEntry(0, offsetof(VulkanUtils::UnlitSingleTexture, buffer1), 0);
                this->AddUpdateEntry(1, offsetof(VulkanUtils::UnlitSingleTexture, buffer2), 0);
                this->AddUpdateEntry(2, offsetof(VulkanUtils::UnlitSingleTexture, buffer3), 0);
                this->AddUpdateEntry(3, offsetof(VulkanUtils::UnlitSingleTexture, texture2D_1), 0);
                m_layoutStructType = EDescriptorLayoutStruct::UnlitSingleTexture;
            }
            else if constexpr(std::is_same_v<t, VulkanUtils::RayTracingDescriptorSet>)
            {
                // Handle UnlitSingleTexture
                // Example: Update descriptor set entries for UnlitSingleTexture
                this->AddUpdateEntry(0, offsetof(VulkanUtils::RayTracingDescriptorSet, buffer1), 0);
                this->AddUpdateEntry(1, offsetof(VulkanUtils::RayTracingDescriptorSet, buffer2), 0);
                this->AddUpdateEntry(2, offsetof(VulkanUtils::RayTracingDescriptorSet, buffer3), 0);
                this->AddUpdateEntry(3, offsetof(VulkanUtils::RayTracingDescriptorSet, tlas), 0);

                this->AddUpdateEntry(4, offsetof(VulkanUtils::RayTracingDescriptorSet, storage2D_1), 0);
                m_layoutStructType = EDescriptorLayoutStruct::RayTracing;

            }
            else if constexpr(std::is_same_v<t, VulkanUtils::ForwardShadingDstSet>)
            {
                this->AddUpdateEntry(0, offsetof(VulkanUtils::ForwardShadingDstSet, buffer1), 0);
                this->AddUpdateEntry(1, offsetof(VulkanUtils::ForwardShadingDstSet, buffer2), 0);
                this->AddUpdateEntry(2, offsetof(VulkanUtils::ForwardShadingDstSet, buffer3), 0);

                this->AddUpdateEntry(3, offsetof(VulkanUtils::ForwardShadingDstSet, buffer4), 0);
                this->AddUpdateEntry(4, offsetof(VulkanUtils::ForwardShadingDstSet, buffer5), 0);
                this->AddUpdateEntry(5, offsetof(VulkanUtils::ForwardShadingDstSet, buffer6), 0);

                this->AddUpdateEntry(6, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_1), 0);
                this->AddUpdateEntry(7, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_2), 0);
                this->AddUpdateEntry(8, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_3), 0);
                this->AddUpdateEntry(9, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_4), 0);
                this->AddUpdateEntry(10, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_5), 0);
                this->AddUpdateEntry(11, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_6), 0);

                // global illumination IBL
                this->AddUpdateEntry(12, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_7), 0);
                this->AddUpdateEntry(13, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_8), 0);
                this->AddUpdateEntry(14, offsetof(VulkanUtils::ForwardShadingDstSet, texture2D_9), 0);
                m_layoutStructType = EDescriptorLayoutStruct::ForwardShading;
            }
            else
            {
                // Handle unexpected type (optional)
                static_assert("Unsupported descriptor set type");
            }
        },
        m_dstLayout->GetStructure());
}

void VulkanUtils::VShaderResrouceGroup::AddUpdateEntry(uint32_t binding, size_t offset, size_t stride)
{
    assert(m_dstLayout->GetBindings().count(binding) == 1 && "Binding is not part of the descriptor layout !");
    vk::DescriptorUpdateTemplateEntry entry{};
    entry.descriptorCount = 1;
    entry.offset          = offset;
    entry.stride          = stride;
    entry.dstBinding      = binding;
    entry.descriptorType  = m_dstLayout->GetBindings()[binding].descriptorType;
    entry.dstArrayElement = 0;

    m_descriptorTemplateEntries.push_back(entry);
}

void VulkanUtils::VShaderResrouceGroup::CreateDstUpdateInfo( const vk::PipelineLayout& pipelineLayout,  vk::PipelineBindPoint bindPoint)
{
    Utils::Logger::LogInfo("Creating update template object....");
    //  assert(!m_descriptorTemplateEntries.empty() && "No template entries found");
    vk::DescriptorUpdateTemplateCreateInfo createInfo{};
    createInfo.descriptorUpdateEntryCount = static_cast<uint32_t>(m_descriptorTemplateEntries.size());
    createInfo.pDescriptorUpdateEntries   = m_descriptorTemplateEntries.data();
    createInfo.templateType               = vk::DescriptorUpdateTemplateType::ePushDescriptors;
    createInfo.descriptorSetLayout        = nullptr;
    createInfo.pipelineBindPoint          = bindPoint;
    createInfo.pipelineLayout             = pipelineLayout;

    m_descriptorUpdateTemplate = m_device.GetDevice().createDescriptorUpdateTemplate(createInfo);

    assert(m_descriptorUpdateTemplate);
    Utils::Logger::LogSuccess("Update template created !");
}

void VulkanUtils::VShaderResrouceGroup::Destroy()
{
    m_device.GetDevice().destroyDescriptorUpdateTemplate(m_descriptorUpdateTemplate);
    m_dstLayout->Destroy();
}
