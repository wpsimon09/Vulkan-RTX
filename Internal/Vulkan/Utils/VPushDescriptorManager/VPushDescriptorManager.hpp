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
#include "VPushDescriptor.hpp"

namespace VulkanCore
{
    class VGraphicsPipeline;
}

namespace VulkanCore
{
    class VDescriptorSetLayout;
    class VDevice;
}



namespace VulkanUtils {


class VPushDescriptorManager {
public:
    explicit VPushDescriptorManager(const VulkanCore::VDevice& device);

    void AddUpdateEntry(uint32_t binding,size_t offset, size_t stride );
    void CreateUpdateTemplate(const VulkanCore::VGraphicsPipeline& pipeline);
    const vk::DescriptorUpdateTemplate& GetTemplate() const {return m_descriptorUpdateTemplate;}

    VulkanUtils::DescriptorSetData& GetDescriptorSetDataStruct() { return m_descriptorSetData; };
    const VulkanCore::VDescriptorSetLayout& GetLayout() const {return *m_descriptorSetLayout;}

    void Destroy();

private:
    const VulkanCore::VDevice& m_device;
    vk::DescriptorUpdateTemplate m_descriptorUpdateTemplate;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_descriptorSetLayout;
    std::vector<vk::DescriptorUpdateTemplateEntry> m_descriptorTemplateEntries;

    VulkanUtils::DescriptorSetData m_descriptorSetData;

    using PushDescriptorVariant =  std::variant<
        std::unique_ptr<VulkanUtils::VPushDescriptorSet<VulkanUtils::BasicDescriptorSet>>,
        std::unique_ptr<VulkanUtils::VPushDescriptorSet<VulkanUtils::ForwardShadingDstSet>>,
        std::unique_ptr<VulkanUtils::VPushDescriptorSet<VulkanUtils::UnlitSingleTexture>>>;

    std::map<EDescriptorLayoutStruct, PushDescriptorVariant> m_pushDescriptors;

};

} // VulkanUtils

#endif //VPUSHDESCRIPTORMANAGER_HPP
