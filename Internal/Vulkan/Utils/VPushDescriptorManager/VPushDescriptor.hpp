//
// Created by wpsimon09 on 18/03/25.
//

#ifndef VPUSHDESCRIPTOR_HPP
#define VPUSHDESCRIPTOR_HPP
#include <memory>
#include <string>
#include <vector>

#include "VDescriptorSetStructs.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"


namespace VulkanCore
{
    class VGraphicsPipeline;
    class VDescriptorSetLayout;
    class VDevice;
}

namespace VulkanUtils {

class VPushDescriptorSet
{
public:
    VPushDescriptorSet(const VulkanCore::VDevice &device,const std::string& name, std::unique_ptr<VulkanCore::VDescriptorSetLayout> dstLayout)
        :m_device(device), m_name(name), m_dstLayout(std::move(dstLayout))
    {};

    DescriptorSetTemplateVariant GetDstStruct()                                {return m_dstLayout->GetStructCopy().value();}
    VulkanCore::VDescriptorSetLayout& GetLayout()                              {return *m_dstLayout; }
    std::string& GetName()                                                     {return m_name;}
    void CreateUpdateEntries(VulkanUtils::DescriptorSetTemplateVariant& teamplate);

    void CreateUpdateEntry(VulkanCore::VGraphicsPipeline& pipelineLayout);

private:
    void AddUpdateEntry(uint32_t binding,size_t offset, size_t stride );
    const VulkanCore::VDevice &m_device;
    std::string m_name;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_dstLayout;
    std::vector<vk::DescriptorUpdateTemplateEntry> m_descriptorTemplateEntries;
    vk::DescriptorUpdateTemplate m_descriptorUpdateTemplate;

};

} // VulkanUtils

#endif //VPUSHDESCRIPTOR_HPP
