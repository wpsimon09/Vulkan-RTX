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
    class VDescriptorSet;
}

namespace VulkanCore
{
    class VDescriptorSetLayout;
    class VBuffer;
    class VDescriptorPool;
    class VDevice;
}

namespace VulkanUtils {

struct DescriptorSet
{

};

class VDescriptorSetManager {
public:
    explicit VDescriptorSetManager(const VulkanCore::VDevice& device);

    // images and other buffers can be passed later as parameters
    void CreateGlobalDescriptorSets(const std::vector<vk::DescriptorBufferInfo>& bufferDescriptorInfo) ;

    const vk::DescriptorSet& GetGlobalDescriptorSet(int imageIndex) const;

    const VulkanCore::VDescriptorSetLayout& GetGlobalDescriptorSetLayout() const;

    void UpdateDescriptorSets(int frameIndex);

    void Destroy();
private:
    const VulkanCore::VDevice& m_device;

    std::unique_ptr<VulkanCore::VDescriptorSet> m_globalDescriptorSet; // per frame in flight

    std::unique_ptr<VulkanCore::VDescriptorPool> m_globalDescriptorPool;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> m_globalDescriptorLayout;
    std::vector<std::unique_ptr<VulkanCore::VDescriptorSet>> m_descriptorWriter;
    std::vector<vk::DescriptorSet> m_globalDescriptorSets;
};

} // VulkanUtils

#endif //VDESCRIPTORSETMANAGER_HPP
