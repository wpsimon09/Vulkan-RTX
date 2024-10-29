//
// Created by wpsimon09 on 29/10/24.
//

#include "VDescriptorSetLayout.hpp"

#include <functional>

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanCore {
    VDescriptorSetLayout::VDescriptorSetLayout(const VulkanCore::VDevice &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& bindings):
        m_device(device), m_descriptorSetLayoutBindings(bindings),VObject() {

        std::vector<vk::DescriptorSetLayoutBinding> setBindings;
        for (auto &binding : m_descriptorSetLayoutBindings) {
            setBindings.push_back(binding.second);
        }

        vk::DescriptorSetLayoutCreateInfo info{};
        info.bindingCount = static_cast<uint32_t>(setBindings.size());
        info.pBindings = setBindings.data();

        m_descriptorSetLayout = m_device.GetDevice().createDescriptorSetLayout(info);
        assert(m_descriptorSetLayout && "Failed to create descriptor set layout");
    }
} // VulkanCore