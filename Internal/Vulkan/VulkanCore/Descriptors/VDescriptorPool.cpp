//
// Created by wpsimon09 on 29/10/24.
//

#include "VDescriptorPool.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VDescriptorPool::Builder::Builder(const VulkanCore::VDevice &device):m_device(device) {
    }

    VDescriptorPool::Builder & VDescriptorPool::Builder::AddPoolsSize(vk::DescriptorType type, uint32_t count) {
        m_poolSizes.push_back({ type, count });
        return *this;
    }

    VDescriptorPool::Builder & VDescriptorPool::Builder::AddPoolFlag(vk::DescriptorPoolCreateFlags flags) {
        m_poolFlags = flags;
        return *this;
    }

    VDescriptorPool::Builder & VDescriptorPool::Builder::AddMaxSets(uint32_t count) {
        m_maxSets = count;
        return *this;
    }

    std::unique_ptr<VDescriptorPool> VDescriptorPool::Builder::Build() const {
        Utils::Logger::LogInfoVerboseOnly("Creating pool to allocate descriptor sets from");
        vk::DescriptorPoolCreateInfo info = {};
        info.flags = m_poolFlags;
        info.maxSets = m_maxSets;
        info.pPoolSizes = m_poolSizes.data();
        auto result = std::make_unique<VDescriptorPool>(m_device, m_maxSets, m_poolFlags, m_poolSizes);
        assert(result);
        Utils::Logger::LogSuccess("Descriptor pool created, buffer can be allocated from");
        return  result;
    }

    VDescriptorPool::VDescriptorPool(const VulkanCore::VDevice &device, uint32_t maxSets,
        vk::DescriptorPoolCreateFlags flags, const std::vector<vk::DescriptorPoolSize> &poolSizes):m_device(device){
        vk::DescriptorPoolCreateInfo info = {};
        info.flags = flags;
        info.maxSets = maxSets;
        info.pPoolSizes = poolSizes.data();
        info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());

        m_descriptorPool = m_device.GetDevice().createDescriptorPool(info);
    }

    void VDescriptorPool::AllocateDescriptor(vk::DescriptorSetLayout layout, vk::DescriptorSet &descriptorSet) const {
        Utils::Logger::LogInfoVerboseOnly("Allocating descriptor set...");
        vk::DescriptorSetAllocateInfo info = {};
        info.descriptorPool = m_descriptorPool;
        info.descriptorSetCount = 1;
        info.pSetLayouts = &layout;
        info.descriptorSetCount = 1;

        descriptorSet = m_device.GetDevice().allocateDescriptorSets(info).front();

        Utils::Logger::LogSuccess("Descriptor set allocated");
    }

    vk::DescriptorSet VDescriptorPool::AllocateDescriptor(vk::DescriptorSetLayout layout) const {
        Utils::Logger::LogInfoVerboseOnly("Allocating descriptor set...");
        vk::DescriptorSetAllocateInfo info = {};
        info.descriptorPool = m_descriptorPool;
        info.descriptorSetCount = 1;
        info.pSetLayouts = &layout;
        info.descriptorSetCount = 1;

        return m_device.GetDevice().allocateDescriptorSets(info).front();

        Utils::Logger::LogSuccess("Descriptor set allocated");
    }

    void VDescriptorPool::FreeDescriptor(std::vector<vk::DescriptorSet> &descriptorSets) {
        m_device.GetDevice().freeDescriptorSets(m_descriptorPool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
    }

    void VDescriptorPool::ResetPool() {
        m_device.GetDevice().resetDescriptorPool(m_descriptorPool);
    }

    void VDescriptorPool::Destroy() {
        m_device.GetDevice().destroyDescriptorPool(m_descriptorPool);
    }
} // VulkanCore