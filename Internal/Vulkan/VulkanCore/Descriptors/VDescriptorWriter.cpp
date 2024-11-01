//
// Created by wpsimon09 on 01/11/24.
//

#include "VDescriptorWriter.hpp"

#include "VDescriptorSetLayout.hpp"

namespace VulkanCore {

    VDescriptorWriter::VDescriptorWriter(const VulkanCore::VDescriptorSetLayout &descriptorSetLayout,
        const VulkanCore::VDescriptorPool &descriptorPool):m_descriptorSetLayout(descriptorSetLayout), m_descriptorPool(descriptorPool) {
    }

    VDescriptorWriter & VDescriptorWriter::WriteBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo) {
        assert(m_descriptorSetLayout.m_descriptorSetLayoutBindings.count(binding) == 1 && "Binidng does not exist in the layout");
        auto &bindingDescription = m_descriptorSetLayout.m_descriptorSetLayoutBindings[binding];
        // we are not writing to array of descriptors therfore we have to have only one descriptor that we are creating write object for
        assert(bindingDescription.descriptorCount == 1 && "Expected more than one binding");
    }

    VDescriptorWriter & VDescriptorWriter::WriteImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo) {
    }

    bool VDescriptorWriter::Build(vk::DescriptorSet &descriptorSet) {
    }

    void VDescriptorWriter::Overwrite(vk::DescriptorSet &descriptorSet) {
    }
} // VulkanCore