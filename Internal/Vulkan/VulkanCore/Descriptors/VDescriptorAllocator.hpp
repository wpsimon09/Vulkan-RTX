//
// Created by wpsimon09 on 10/05/25.
//

#ifndef VDESCRIPTORALLOCATOR_HPP
#define VDESCRIPTORALLOCATOR_HPP

#include "Vulkan/VulkanCore/VObject.hpp"
#include "vulkan/vulkan.hpp"

#include <unordered_map>
#include <vector>
// thin abstraction over descriptor sets based on  https://github.com/vblanco20-1/Vulkan-Descriptor-Allocator

namespace VulkanCore {
class VDevice;

/**
 * Class used to allocate descriptors from the provided layout, under the hood it also manages descriptor pool craetion and management.
 */
class VDescriptorAllocator : public VulkanCore::VObject
{
  public:
    VDescriptorAllocator(const VDevice& device);

    struct PoolSizes
    {
        std::vector<std::pair<vk::DescriptorType, float>> sizes = {{vk::DescriptorType::eSampler, 0.5f},
                                                                   {vk::DescriptorType::eCombinedImageSampler, 4.f},
                                                                   {vk::DescriptorType::eSampledImage, 4.f},
                                                                   {vk::DescriptorType::eStorageImage, 1.f},
                                                                   {vk::DescriptorType::eUniformTexelBuffer, 1.f},
                                                                   {vk::DescriptorType::eStorageTexelBuffer, 1.f},
                                                                   {vk::DescriptorType::eUniformBuffer, 2.f},
                                                                   {vk::DescriptorType::eStorageBuffer, 2.f},
                                                                   {vk::DescriptorType::eUniformBufferDynamic, 1.f},
                                                                   {vk::DescriptorType::eStorageBufferDynamic, 1.f},
                                                                   {vk::DescriptorType::eInputAttachment, 0.5f},
                                                                   {vk::DescriptorType::eAccelerationStructureKHR, 0.001}};
    };
    void Destroy() override;

    void ResetPools();
    bool Allocate(vk::DescriptorSet* set, vk::DescriptorSetLayout dLayout);

  private:
    vk::DescriptorPool GrabPool();

  private:
    vk::DescriptorPool         m_currentPool;
    const VulkanCore::VDevice& m_device;

    PoolSizes                       m_descriptorSizes;
    std::vector<vk::DescriptorPool> m_freePools;
    std::vector<vk::DescriptorPool> m_usedPools;
};

/**
 * Class that handles descriptor layout caching, in case exact same layout is about to be created, this class will look for allready existing layout
 * and return it instead of crating a new desriptor set layout.
 */
class VDescriptorLayoutCache : public VObject
{
  public:
    explicit VDescriptorLayoutCache(const VDevice& device);

    void Destroy() override;

    vk::DescriptorSetLayout CreateDescriptorSetLayout(const vk::DescriptorSetLayoutCreateInfo* info);

    struct DescriptorSetLayoutInfo
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        bool                                        operator==(const DescriptorSetLayoutInfo& other) const;

        size_t hash() const;
    };


  private:
    void ValidateCreateInfo(const vk::DescriptorSetLayoutCreateInfo* info);
    struct DescriptorLayoutHash
    {
        std::size_t operator()(const DescriptorSetLayoutInfo& k) const { return k.hash(); }
    };

    std::unordered_map<DescriptorSetLayoutInfo, vk::DescriptorSetLayout, DescriptorLayoutHash> m_layoutCache;

    const VDevice& m_device;
};

/**
 * This class will build the descriptor set layouts and and allocate them from the pool(s)
 */
class VDescriptorBuilder
{
  public:
    static VDescriptorBuilder Begin(VDescriptorLayoutCache* layoutCache, VDescriptorAllocator* allocator);

    bool Build(vk::DescriptorSet& set, vk::DescriptorSetLayout& layout);
    bool Build(vk::DescriptorSet& set);

  private:
    std::vector<vk::WriteDescriptorSet>         m_descriptorWrites;
    std::vector<vk::DescriptorSetLayoutBinding> m_layoutBindings;

    VDescriptorLayoutCache* m_descLayoutCache = nullptr;
    VDescriptorAllocator*   m_descAllocator = nullptr;


  private:
};


}  // namespace VulkanCore

#endif  //VDESCRIPTORALLOCATOR_HPP
