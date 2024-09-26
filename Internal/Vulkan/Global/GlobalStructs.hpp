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
        Utils::Logger::LogInfoVerboseOnly("Checking device..." );
        if(deviceType != this->deviceType) {
            Utils::Logger::LogInfoVerboseOnly("Device type miss match");
            return false;
        }
        if(deviceFeatures.geometryShader != this->deviceFeatures.geometryShader) {
            Utils::Logger::LogInfoVerboseOnly("Device does not support geometry shader");
            return false;
        }
        Utils::Logger::LogInfoVerboseOnly("Device has passed all the checks");
        return true;
    }
};


namespace GlobalVariables::GlobalStructs
{
    inline extern DesiredDeviceFeatures primaryDeviceFeatures = {
        .deviceType = vk::PhysicalDeviceType::eDiscreteGpu,
        .deviceFeatures = vk::PhysicalDeviceFeatures{}.setGeometryShader(VK_TRUE)
    };

    inline extern DesiredDeviceFeatures secondaryDeviceFeatures = {
        .deviceType = vk::PhysicalDeviceType::eIntegratedGpu,
        .deviceFeatures = vk::PhysicalDeviceFeatures{}
    };
}

#endif //GLOBALSTRUCTS_HPP
