//
// Created by wpsimon09 on 29/10/24.
//

#ifndef VDESCRIPTORSETLAYOUT_HPP
#define VDESCRIPTORSETLAYOUT_HPP
#include <unordered_map>

#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>

namespace VulkanCore {
    class VDevice;

    class VDescriptorSetLayout: VObject {
public:
    explicit VDescriptorSetLayout(const VulkanCore::VDevice& descriptorSetLayout, std::unordered_map<uint32_t,vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings);


};

} // VulkanCore

#endif //VDESCRIPTORSETLAYOUT_HPP
