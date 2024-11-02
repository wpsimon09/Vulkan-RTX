//
// Created by wpsimon09 on 02/11/24.
//

#ifndef VDESCRIPTORSETMANAGER_HPP
#define VDESCRIPTORSETMANAGER_HPP
#include <memory>
#include <vector>
#include "vulkan/vulkan.hpp"

namespace VulkanCore
{
    class VDescriptorSetLayout;
    class VBuffer;
    class VDescriptorPool;
    class VDevice;
}

namespace VulkanUtils {

class VDescriptorSetManager {
public:
    explicit VDescriptorSetManager(const VulkanCore::VDevice& device);

    // images and other buffers can be passed later as parameters
    void CreateGlobalDescriptorSets(std::vector<vk::DescriptorBufferInfo *>& bufferDescriptorInfo) ;

    void Destroy();
private:
    const VulkanCore::VDevice& m_device;
    std::unique_ptr<VulkanCore::VDescriptorPool> m_descriptorPoolGlobal;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_globalDescriptorLayout;
    std::vector<vk::DescriptorSet> m_globalDescriptorSets;
};

} // VulkanUtils

#endif //VDESCRIPTORSETMANAGER_HPP
