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
    explicit VPushDescriptorSet(const VulkanCore::VDevice &device,const std::string& name, std::unique_ptr<VulkanCore::VDescriptorSetLayout> dstLayout);



    DescriptorSetTemplateVariant& GetDstStruct()                               {return m_dstLayout->GetStructure();}
    VulkanCore::VDescriptorSetLayout& GetLayout()                              {return *m_dstLayout; }
    std::string& GetName()                                                     {return m_name;}

    void CreateDstUpdateInfo(VulkanCore::VGraphicsPipeline& pipelineLayout);

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
