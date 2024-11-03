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

class VDescriptorWriter:public VObject {
public:
    explicit VDescriptorWriter(VulkanCore::VDescriptorSetLayout& descriptorSetLayout, const VulkanCore::VDescriptorPool& descriptorPool );

    VDescriptorWriter& WriteBuffer(uint32_t binding, vk::DescriptorBufferInfo bufferInfo );
    VDescriptorWriter& WriteImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo );

    void Build(vk::DescriptorSet &descriptorSet) const;
    void Overwrite(const vk::DescriptorSet &descriptorSet);

private:
    VulkanCore::VDescriptorSetLayout& m_descriptorSetLayout;
    const VulkanCore::VDescriptorPool& m_descriptorPool;
    std::vector<vk::WriteDescriptorSet> m_descriptorWrites;
};

} // VulkanCore

#endif //VDESCRIPTORWRITER_HPP
