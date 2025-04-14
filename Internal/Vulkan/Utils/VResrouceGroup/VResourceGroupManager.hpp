//
// Created by wpsimon09 on 08/11/24.
//

#ifndef VPUSHDESCRIPTORMANAGER_HPP
#define VPUSHDESCRIPTORMANAGER_HPP
#include <map>
#include <memory>
#include <vector>

#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "vulkan/vulkan.hpp"
#include "VDescriptorSetStructs.hpp"
#include "VResrouceGroup.hpp"

namespace VulkanCore {
class VGraphicsPipeline;
}

namespace VulkanCore {
class VDescriptorSetLayout;
class VDevice;
}  // namespace VulkanCore


namespace VulkanUtils {


class VResourceGroupManager
{
  public:
    explicit VResourceGroupManager(const VulkanCore::VDevice& device);

    void AddUpdateEntry(uint32_t binding, size_t offset, size_t stride);
    void CreateUpdateTemplate(const VulkanCore::VGraphicsPipeline& pipeline);

    std::shared_ptr<VShaderResrouceGroup>& GetPushDescriptor(EDescriptorLayoutStruct layoutType)
    {
        return m_pushDescriptors[layoutType];
    }

    VulkanUtils::DescriptorSetData& GetDescriptorSetDataStruct() { return m_descriptorSetData; };

    void Destroy();

  private:
    const VulkanCore::VDevice&                     m_device;
    std::vector<vk::DescriptorUpdateTemplateEntry> m_descriptorTemplateEntries;

    VulkanUtils::DescriptorSetData m_descriptorSetData;

    std::map<EDescriptorLayoutStruct, std::shared_ptr<VulkanUtils::VShaderResrouceGroup>> m_pushDescriptors;
};

}  // namespace VulkanUtils

#endif  //VPUSHDESCRIPTORMANAGER_HPP
