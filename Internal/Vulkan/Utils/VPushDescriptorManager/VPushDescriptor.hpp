//
// Created by wpsimon09 on 18/03/25.
//

#ifndef VPUSHDESCRIPTOR_HPP
#define VPUSHDESCRIPTOR_HPP
#include <memory>
#include <string>
#include <vector>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace VulkanUtils {

template<typename T>
class VPushDescriptorSet
{
public:
    VPushDescriptorSet(const VulkanCore::VDevice &device,std::string& name, std::unique_ptr<VulkanCore::VDescriptorSetLayout> dstLayout,T& dstStruct)
        :m_device(device), m_dstStruct(dstStruct), m_name(name), m_dstLayout(std::move(dstLayout))
    {};

    T& GetDstStruct() {return m_dstStruct;};
    VulkanCore::VDescriptorSetLayout& GetLayout(){return *m_dstLayout; }
    std::string& GetName(){return m_name;};

    void AddUpdateEntry(uint32_t binding,size_t offset, size_t stride );
    void CreateUpdateEnetry(VulkanCore::VGraphicsPipeline& pipelineLayout);

private:
    const VulkanCore::VDevice &m_device;
    T m_dstStruct;
    std::string m_name;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_dstLayout;
    std::vector<vk::DescriptorUpdateTemplateEntry> m_descriptorTemplateEntries;
    vk::DescriptorUpdateTemplate m_descriptorUpdateTemplate;

};

//======================================================================================================
//======================================================================================================
// IMPLEMENTATION
//======================================================================================================
//======================================================================================================

template <typename T>
void VPushDescriptorSet<T>::AddUpdateEntry(uint32_t binding, size_t offset, size_t stride)
{
    assert(m_dstLayout->GetBindings().count(binding) == 1 && "Binding is not part of the descriptor layout !");
    vk::DescriptorUpdateTemplateEntry entry{};
    entry.descriptorCount = 1;
    entry.offset = offset;
    entry.stride = stride;
    entry.dstBinding = binding;
    entry.descriptorType = m_dstLayout->GetBindings()[binding].descriptorType;
    entry.dstArrayElement = 0;

    m_descriptorTemplateEntries.push_back(entry);
}

template <typename T>
void VPushDescriptorSet<T>::CreateUpdateEnetry(VulkanCore::VGraphicsPipeline& pipelineLayout)
{
    Utils::Logger::LogInfo("Creating update template object....");
    assert(!m_descriptorTemplateEntries.empty() && "No template entries found");
    vk::DescriptorUpdateTemplateCreateInfo createInfo{};
    createInfo.descriptorUpdateEntryCount = static_cast<uint32_t>(m_descriptorTemplateEntries.size());
    createInfo.pDescriptorUpdateEntries = m_descriptorTemplateEntries.data();
    createInfo.templateType =  vk::DescriptorUpdateTemplateType::ePushDescriptors;
    createInfo.descriptorSetLayout =nullptr;
    createInfo.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    createInfo.pipelineLayout = pipelineLayout.GetPipelineLayout();

    m_descriptorUpdateTemplate = m_device.GetDevice().createDescriptorUpdateTemplate(createInfo);

    assert(m_descriptorUpdateTemplate);
    Utils::Logger::LogSuccess(  "Update template created !");
}

} // VulkanUtils

#endif //VPUSHDESCRIPTOR_HPP
