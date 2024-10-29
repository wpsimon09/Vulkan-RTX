//
// Created by wpsimon09 on 29/10/24.
//

#ifndef VDESCRIPTORSETLAYOUT_HPP
#define VDESCRIPTORSETLAYOUT_HPP
#include <memory>
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
        const vk::DescriptorSetLayout& GetDescriptorSetLayout()  const {return m_descriptorSetLayout;};

    private:
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& m_descriptorSetLayoutBindings;
        const VDevice& m_device;
        vk::DescriptorSetLayout m_descriptorSetLayout;
    };

    class VDescriptorSetLayoutBuilder{
    public:
        explicit VDescriptorSetLayoutBuilder(const VulkanCore::VDevice& device);
        VDescriptorSetLayoutBuilder& AddBinding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stage, uint32_t descriptorCount) ;
        std::unique_ptr<VulkanCore::VDescriptorSetLayout> Build();
    private:
        std::unordered_map<uint32_t,vk::DescriptorSetLayoutBinding> m_descriptorBindings;
        const VulkanCore::VDevice& m_device;
    };

} // VulkanCore

#endif //VDESCRIPTORSETLAYOUT_HPP
