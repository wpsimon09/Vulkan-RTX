//
// Created by wpsimon09 on 08/11/24.
//

#ifndef VPUSHDESCRIPTORMANAGER_HPP
#define VPUSHDESCRIPTORMANAGER_HPP
#include <memory>
#include <vector>

#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "vulkan/vulkan.hpp"

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

struct DescriptorSetData
{
    vk::DescriptorBufferInfo cameraUBOBuffer; // for camera uniform buffer
    vk::DescriptorBufferInfo meshUBBOBuffer; // for mesh uniform buffer
};

class VPushDescriptorManager {
public:
    explicit VPushDescriptorManager(const VulkanCore::VDevice& device);

    void AddBufferEntry(uint32_t binding,size_t offset, size_t stride );
    void AddImageEntry(uint32_t binding, vk::DescriptorImageInfo* imageInfo ){};
    void CreateUpdateTemplate(const VulkanCore::VGraphicsPipeline& pipeline);
    const vk::DescriptorUpdateTemplate& GetTemplate() const {return m_descriptorUpdateTemplate;}

    DescriptorSetData& GetDescriptorSetDataStruct() { return m_descriptorSetData; };
    const VulkanCore::VDescriptorSetLayout& GetLayout() const {return *m_descriptorSetLayout;}

    void Destroy();

private:
    const VulkanCore::VDevice& m_device;
    vk::DescriptorUpdateTemplate m_descriptorUpdateTemplate;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_descriptorSetLayout;
    std::vector<vk::DescriptorUpdateTemplateEntry> m_descriptorTemplateEntries;

    DescriptorSetData m_descriptorSetData;
};

} // VulkanUtils

#endif //VPUSHDESCRIPTORMANAGER_HPP
