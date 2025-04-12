//
// Created by wpsimon09 on 26/09/24.
//

#ifndef GLOBALSTRUCTS_HPP
#define GLOBALSTRUCTS_HPP

#include <vulkan/vulkan.hpp>

#include "Vulkan/Utils/VChecker.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Application/Logger/Logger.hpp"

struct DesiredDeviceFeatures
{
    vk::PhysicalDeviceType deviceType;
    vk::PhysicalDeviceFeatures deviceFeatures;

    bool CheckAgainstRetrievedPhysicalDevice(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) const {
        Utils::Logger::LogInfoVerboseOnly("Checking device..." );
        if(physicalDevice.getProperties().deviceType != this->deviceType) {
            Utils::Logger::LogInfoVerboseOnly("Device type miss match");
            return false;
        }
        if(physicalDevice.getFeatures().geometryShader != this->deviceFeatures.geometryShader ) {
            Utils::Logger::LogInfoVerboseOnly("Device does not support geometry shader");
            return false;
        }
        if(!VulkanUtils::DoesDeviceSupportPresentation(surface, physicalDevice)) {
            Utils::Logger::LogInfoVerboseOnly("Device does not supports presentation");
            return false;
        }

        Utils::Logger::LogInfoVerboseOnly("Device has passed all the checks");
        return true;
    }
};


namespace GlobalVariables::GlobalStructs
{
    inline DesiredDeviceFeatures primaryDeviceFeatures = {
        .deviceType = vk::PhysicalDeviceType::eDiscreteGpu,
        .deviceFeatures =
            vk::PhysicalDeviceFeatures{}
                .setGeometryShader(VK_TRUE)
                .setSamplerAnisotropy(VK_TRUE),
    };

    inline DesiredDeviceFeatures secondaryDeviceFeatures = {
        .deviceType = vk::PhysicalDeviceType::eIntegratedGpu,
        .deviceFeatures = vk::PhysicalDeviceFeatures{}
                        .setSamplerAnisotropy(VK_TRUE)



    };

    inline vk::PhysicalDeviceProperties2 GpuProperties = {};
    inline vk::PhysicalDeviceMemoryProperties2 GpuMemoryProperties = {};
    inline vk::PhysicalDeviceAccelerationStructurePropertiesKHR AccelerationStructProperties = {};
    inline vk::PhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPipelineProperties = {};

}

#endif //GLOBALSTRUCTS_HPP
