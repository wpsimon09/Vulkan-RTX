//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VCHECKER_HPP
#define VCHECKER_HPP

#include<vector>
namespace vk
{
    class PhysicalDevice;
}

namespace VulkanUtils
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    bool CheckValidationLayerSupport();

    bool CheckPhysicalDeviceCompatibility(vk::PhysicalDevice physicalDevice);
};


#endif //VCHECKER_HPP