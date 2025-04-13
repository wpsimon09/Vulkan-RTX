//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VCHECKER_HPP
#define VCHECKER_HPP

#include <vector>

#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace vk {
class PhysicalDevice;
}

namespace VulkanUtils {
const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

bool CheckValidationLayerSupport();

bool DoesDeviceSupportPresentation(const vk::SurfaceKHR& surface, const vk::PhysicalDevice& physicalDevice);
};  // namespace VulkanUtils


#endif  //VCHECKER_HPP
