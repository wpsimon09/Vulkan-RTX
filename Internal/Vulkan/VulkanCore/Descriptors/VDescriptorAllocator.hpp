//
// Created by wpsimon09 on 10/05/25.
//

#ifndef VDESCRIPTORALLOCATOR_HPP
#define VDESCRIPTORALLOCATOR_HPP

#include "Vulkan/VulkanCore/VObject.hpp"
#include "vulkan/vulkan.hpp"

#include <vector>
// thin abstraction over descriptor sets based on  https://github.com/vblanco20-1/Vulkan-Descriptor-Allocator

namespace VulkanCore {
class VDevice;

class VDescriptorAllocator: public VulkanCore::VObject
{
  public:
    VDescriptorAllocator(const VDevice& device);

    struct PoolSizes
    {
        std::vector<std::pair<vk::DescriptorType, float>> sizes = {{vk::DescriptorType::eSampler, 0.5f},
                                                                   {vk::DescriptorType::eCombinedImageSampler, 4.f},
                                                                   {vk::DescriptorType::eSampledImage, 4.f},
                                                                   {vk::DescriptorType::eSampledImage, 1.f},
                                                                   {vk::DescriptorType::eUniformTexelBuffer, 1.f},
                                                                   {vk::DescriptorType::eStorageTexelBuffer, 1.f},
                                                                   {vk::DescriptorType::eUniformBuffer, 2.f},
                                                                   {vk::DescriptorType::eStorageBuffer, 2.f},
                                                                   {vk::DescriptorType::eUniformBufferDynamic, 1.f},
                                                                   {vk::DescriptorType::eStorageBufferDynamic, 1.f},
                                                                   {vk::DescriptorType::eInputAttachment, 0.5f}};
    };
    void Destroy() override;

    void ResetPools();
    bool Allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout dLayout);
private:
    vk::DescriptorPool GrabPool();
  private:
    vk::DescriptorPool m_currentPool;
    const VulkanCore::VDevice& m_device;

    PoolSizes m_descriptorSizes;
    std::vector<vk::DescriptorPool> m_freePools;
    std::vector<vk::DescriptorPool> m_usedPools;

};

class vDescriptorLayoutCache
{
};


}  // namespace VulkanCore

#endif  //VDESCRIPTORALLOCATOR_HPP
