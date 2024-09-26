//
// Created by wpsimon09 on 26/09/24.
//

#ifndef GLOBALSTRUCTS_HPP
#define GLOBALSTRUCTS_HPP

#include <vulkan/vulkan.hpp>

struct DesiredDeviceFeatures
{
    vk::PhysicalDeviceType deviceType;
    vk::PhysicalDeviceFeatures deviceFeatures;

    bool CheckAgainstRetrievedPhysicalDevice(vk::PhysicalDeviceType deviceType, vk::PhysicalDeviceFeatures deviceFeatures) {
        if(deviceType != this->deviceType) return false;
        if(deviceFeatures.geometryShader != this->deviceFeatures.geometryShader) return false;
    }
};

namespace GlobalVariables::GlobalStructs
{
    inline extern DesiredDeviceFeatures primaryDeviceFeatures = {
        .deviceType = vk::PhysicalDeviceType::eDiscreteGpu,
        .deviceFeatures = {
            .geometryShader = true,
        }
    };
    inline extern DesiredDeviceFeatures secondaryDeviceFeatures = {
        .deviceType = vk::PhysicalDeviceType::eCpu,
        .deviceFeatures = {}
    };
}

#endif //GLOBALSTRUCTS_HPP
