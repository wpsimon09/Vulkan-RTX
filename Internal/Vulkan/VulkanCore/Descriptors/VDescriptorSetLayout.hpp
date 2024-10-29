//
// Created by wpsimon09 on 29/10/24.
//

#ifndef VDESCRIPTORSETLAYOUT_HPP
#define VDESCRIPTORSETLAYOUT_HPP
#include <unordered_map>

#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VDevice;

    class VDescriptorSetLayout : VObject
    {
    public:
        explicit VDescriptorSetLayout(const VulkanCore::VDevice &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& bindings);

    private:
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& m_descriptorSetLayoutBindings;
        const VDevice& m_device;
        vk::DescriptorSetLayout m_descriptorSetLayout;
    };

} // VulkanCore

#endif //VDESCRIPTORSETLAYOUT_HPP
