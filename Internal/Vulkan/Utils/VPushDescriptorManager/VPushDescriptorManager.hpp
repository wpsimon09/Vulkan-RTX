//
// Created by wpsimon09 on 08/11/24.
//

#ifndef VPUSHDESCRIPTORMANAGER_HPP
#define VPUSHDESCRIPTORMANAGER_HPP
#include <memory>
#include <vector>

#include "vulkan/vulkan.hpp"
namespace VulkanCore
{
    class VDescriptorSetLayout;
    class VDevice;
}

namespace VulkanUtils {

class VPushDescriptorManager {
    explicit VPushDescriptorManager(const VulkanCore::VDevice& device);
    void WriteBuffer(uint32_t binding,vk::DescriptorBufferInfo &bufferInfo );
    void WriteImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo );
    std::vector<vk::WriteDescriptorSet>& GetWriteDescriptorSets() {return m_writeDescriptorSets;};


private:
    const VulkanCore::VDevice& m_device;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_descriptorSetLayout;
    std::vector<vk::WriteDescriptorSet> m_writeDescriptorSets;
};

} // VulkanUtils

#endif //VPUSHDESCRIPTORMANAGER_HPP
