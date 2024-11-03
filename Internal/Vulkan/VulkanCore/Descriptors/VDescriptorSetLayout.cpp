//
// Created by wpsimon09 on 29/10/24.
//

#include "VDescriptorSetLayout.hpp"

#include <functional>

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VDescriptorSetLayout::VDescriptorSetLayout(const VulkanCore::VDevice &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings):
        m_device(device), m_descriptorSetLayoutBindings(std::move(bindings)),VObject() {

        Utils::Logger::LogInfoVerboseOnly("Creating descriptor set layout");
        std::vector<vk::DescriptorSetLayoutBinding> setBindings;
        for (auto &binding : m_descriptorSetLayoutBindings) {
            setBindings.push_back(binding.second);
        }

        vk::DescriptorSetLayoutCreateInfo info{};
        info.bindingCount = static_cast<uint32_t>(setBindings.size());
        info.pBindings = setBindings.data();

        m_descriptorSetLayout = m_device.GetDevice().createDescriptorSetLayout(info);
        assert(m_descriptorSetLayout && "Failed to create descriptor set layout");

        Utils::Logger::LogSuccess("Descriptor set layout created");
    }

    void VDescriptorSetLayout::Destroy() {
        m_device.GetDevice().destroyDescriptorSetLayout(m_descriptorSetLayout);
    }

    VDescriptorSetLayout::Builder::Builder(const VulkanCore::VDevice &device):m_device(device) { }

    VDescriptorSetLayout::Builder & VDescriptorSetLayout::Builder::AddBinding(uint32_t binding, vk::DescriptorType type,
        vk::ShaderStageFlags stage, uint32_t descriptorCount) {

        assert(m_descriptorBindings.count(binding) == 0 && "Binding already exists");

        vk::DescriptorSetLayoutBinding layout{};
        layout.binding = binding;
        layout.descriptorType = type;
        layout.descriptorCount = descriptorCount;
        layout.stageFlags = stage;
        m_descriptorBindings[binding] = layout;

        return *this;
    }

    std::unique_ptr<VulkanCore::VDescriptorSetLayout> VDescriptorSetLayout::Builder::Build() {
        return std::make_unique<VDescriptorSetLayout>(m_device, m_descriptorBindings);
    }
} // VulkanCore