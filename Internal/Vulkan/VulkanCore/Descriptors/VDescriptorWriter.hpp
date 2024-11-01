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
    explicit VDescriptorWriter(const VulkanCore::VDescriptorSetLayout& descriptorSetLayout, const VulkanCore::VDescriptorPool& descriptorPool );

    VDescriptorWriter& WriteBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo );
    VDescriptorWriter& WriteImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo );

    bool Build(vk::DescriptorSet &descriptorSet);
    void Overwrite(vk::DescriptorSet &descriptorSet);

private:
    const VulkanCore::VDescriptorSetLayout& descriptorSetLayout;
    const VulkanCore::VDescriptorPool& descriptorPool;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;
};

} // VulkanCore

#endif //VDESCRIPTORWRITER_HPP
