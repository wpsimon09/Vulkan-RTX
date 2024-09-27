//
// Created by wpsimon09 on 27/09/24.
//

#ifndef VGENERALUTILS_HPP
#define VGENERALUTILS_HPP
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace VulkanUtils
{
    uint32_t FindQueueFamily(const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties, vk::QueueFlagBits queueType);
}

#endif //VGENERALUTILS_HPP
