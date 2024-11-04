//
// Created by wpsimon09 on 01/11/24.
//

#include "VDescriptorWriter.hpp"

#include "VDescriptorPool.hpp"
#include "VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {

    VDescriptorWriter::VDescriptorWriter(VulkanCore::VDescriptorSetLayout &descriptorSetLayout,
        const VulkanCore::VDescriptorPool &descriptorPool):m_descriptorSetLayout(descriptorSetLayout), m_descriptorPool(descriptorPool) {
    }

    VDescriptorWriter & VDescriptorWriter::WriteBuffer(uint32_t binding,const vk::DescriptorBufferInfo& bufferInfo) {
        Utils::Logger::LogInfoVerboseOnly("Creating writable descriptor objects...");

        assert(m_descriptorSetLayout.m_descriptorSetLayoutBindings.count(binding) == 1);
        auto &bindingDescription = m_descriptorSetLayout.m_descriptorSetLayoutBindings[binding];
        // we are not writing to array of descriptors therfore we have to have only one descriptor that we are creating write object for
        assert(bindingDescription.descriptorCount == 1 && "Expected more than one binding");
        vk::WriteDescriptorSet descriptorWrite = {};
        descriptorWrite.descriptorType = m_descriptorSetLayout.m_descriptorSetLayoutBindings[binding].descriptorType;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.descriptorCount = 1;

        m_descriptorWrites.push_back(descriptorWrite);
        Utils::Logger::LogSuccess("Writable buffer created !");
        return *this;
    }

    VDescriptorWriter & VDescriptorWriter::WriteImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo) {
        return *this;
    }

    void VDescriptorWriter::Build(vk::DescriptorSet &descriptorSet) const {
        Utils::Logger::LogInfoVerboseOnly("Creating descriptor set...");
        m_descriptorPool.AllocateDescriptor(m_descriptorSetLayout.GetLayout(), descriptorSet);
        Utils::Logger::LogSuccess("Descriptor set created !");
    }

    void VDescriptorWriter::Overwrite(const vk::DescriptorSet &descriptorSet) {

        for(auto& write: m_descriptorWrites) {
            write.dstSet = descriptorSet;
        }
        m_descriptorPool.m_device.GetDevice().updateDescriptorSets(m_descriptorWrites.size(), m_descriptorWrites.data(),0,nullptr);
    }
} // VulkanCore