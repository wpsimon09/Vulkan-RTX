//
// Created by wpsimon09 on 28/10/24.
//

#include "VDescriptorSetLayoutBuilder.hpp"

#include "VDescriptorSetLayout.hpp"

namespace VulkanCore {
    VDescriptorSetLayoutBuilder::VDescriptorSetLayoutBuilder(const VulkanCore::VDevice &device):m_device(device) {
    }

    VDescriptorSetLayoutBuilder & VDescriptorSetLayoutBuilder::AddBinding(uint32_t binding, vk::DescriptorType type,
        vk::ShaderStageFlags stage, uint32_t descriptorCount) {

        assert(m_descriptorBindings.count(binding) == 0 && "Binidng allready exists");

        vk::DescriptorSetLayoutBinding layout{};
        layout.binding = binding;
        layout.descriptorType = type;
        layout.descriptorCount = descriptorCount;
        layout.stageFlags = stage;
        m_descriptorBindings[binding] = layout;

        return *this;
    }

    std::unique_ptr<VulkanCore::VDescriptorSetLayout> VDescriptorSetLayoutBuilder::Build() {
        return std::make_unique<VDescriptorSetLayout>(m_device, m_descriptorBindings);
    }
} // VulkanCore