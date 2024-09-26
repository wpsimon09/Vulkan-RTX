//
// Created by wpsimon09 on 25/09/24.
//

#include "VDevice.hpp"

#include "Includes/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"

VulkanCore::VDevice::VDevice(const vk::Instance &instance):m_insatnce(instance) {
    m_physicalDevice = PickPhysicalDevice();
}

vk::PhysicalDevice VulkanCore::VDevice::PickPhysicalDevice() {
    auto availablePhysicalDevices = m_insatnce.enumeratePhysicalDevices();
    for(auto  &physicalDevice: availablePhysicalDevices) {
        Utils::Logger::LogInfo("Found physical device: " + std::string(physicalDevice.getProperties().deviceName));
        if(GlobalVariables::GlobalStructs::primaryDeviceFeatures.CheckAgainstRetrievedPhysicalDevice(physicalDevice.getProperties().deviceType, physicalDevice.getFeatures())){
            return physicalDevice;
        }else if(GlobalVariables::GlobalStructs::primaryDeviceFeatures.CheckAgainstRetrievedPhysicalDevice(physicalDevice.getProperties().deviceType, physicalDevice.getFeatures())){
            return physicalDevice;
        }else {
            throw std::runtime_error("Could not find a valid physical device try to disable some features");
        }
    }
}
