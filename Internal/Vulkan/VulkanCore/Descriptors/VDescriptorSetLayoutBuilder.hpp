//
// Created by wpsimon09 on 28/10/24.
//

#ifndef VDESCRIPTORSETLAYOUTBUILDER_HPP
#define VDESCRIPTORSETLAYOUTBUILDER_HPP
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace VulkanCore {
    class VDescriptorSetLayout;
    class VDevice;
class VDescriptorSetLayoutBuilder{
    public:
    explicit VDescriptorSetLayoutBuilder(const VulkanCore::VDevice& device);
    VDescriptorSetLayoutBuilder& AddBinding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stage, uint32_t descriptorCount) ;
    std::unique_ptr<VulkanCore::VDescriptorSetLayout> Build();
    private:
        std::unordered_map<uint32_t,vk::DescriptorSetLayoutBinding> m_descriptorBindings;
        const VulkanCore::VDevice& m_device
    ;
};

} // VulkanCore

#endif //VDESCRIPTORSETLAYOUTBUILDER_HPP
