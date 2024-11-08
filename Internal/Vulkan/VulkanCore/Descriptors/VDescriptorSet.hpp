//
// Created by wpsimon09 on 01/11/24.
//

#ifndef VDESCRIPTORWRITER_HPP
#define VDESCRIPTORWRITER_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include "vulkan/vulkan.hpp"

namespace VulkanCore {

class VDescriptorSetLayout;
class VDescriptorPool;

class VDescriptorSet:public VObject {
public:
    explicit VDescriptorSet(VulkanCore::VDescriptorSetLayout& descriptorSetLayout, const VulkanCore::VDescriptorPool& descriptorPool );
    VDescriptorSet& WriteBuffer(uint32_t binding, const vk::DescriptorBufferInfo &bufferInfo );
    VDescriptorSet& WriteImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo );

    static  vk::WriteDescriptorSet WriteBuffer(uint32_t binding, uint32_t set,vk::DescriptorType dstType, vk::DescriptorBufferInfo& bufferInfo);

    void Build(vk::DescriptorSet &descriptorSet) const;
    void Overwrite(const vk::DescriptorSet &descriptorSet);

private:
    VulkanCore::VDescriptorSetLayout& m_descriptorSetLayout;
    const VulkanCore::VDescriptorPool& m_descriptorPool;
    std::vector<vk::WriteDescriptorSet> m_descriptorWrites;
    std::vector<vk::DescriptorSet> m_descriptorSets; // per frame in flight
};

} // VulkanCore

#endif //VDESCRIPTORWRITER_HPP
