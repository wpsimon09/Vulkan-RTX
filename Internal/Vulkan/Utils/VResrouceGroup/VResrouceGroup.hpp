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


namespace VulkanCore {
class VGraphicsPipeline;
class VDescriptorSetLayout;
class VDevice;
}  // namespace VulkanCore

namespace VulkanUtils {

class VShaderResrouceGroup
{
  public:
    explicit VShaderResrouceGroup(const VulkanCore::VDevice&                        device,
                                  const std::string&                                name,
                                  std::unique_ptr<VulkanCore::VDescriptorSetLayout> dstLayout);


    DescriptorSetTemplateVariant&     GetResourceGroupStruct() { return m_dstLayout->GetStructure(); }
    VulkanCore::VDescriptorSetLayout& GetDescriptorSetLayout() { return *m_dstLayout; }
    std::string&                      GetName() { return m_name; }
    EDescriptorLayoutStruct           GetResourceGroupStrucutureType() { return m_layoutStructType; }

    void                          CreateDstUpdateInfo(VulkanCore::VGraphicsPipeline& pipelineLayout);
    vk::DescriptorUpdateTemplate& GetUpdateTemplate() { return m_descriptorUpdateTemplate; }
    void                          Destroy();

  protected:
    EDescriptorLayoutStruct                           m_layoutStructType;
    void                                              AddUpdateEntry(uint32_t binding, size_t offset, size_t stride);
    const VulkanCore::VDevice&                        m_device;
    std::string                                       m_name;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_dstLayout;
    std::vector<vk::DescriptorUpdateTemplateEntry>    m_descriptorTemplateEntries;
    vk::DescriptorUpdateTemplate                      m_descriptorUpdateTemplate;
};

}  // namespace VulkanUtils

#endif  //VPUSHDESCRIPTOR_HPP
