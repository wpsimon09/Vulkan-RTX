//
// Created by wpsimon09 on 25/09/24.
//

#include "VDevice.hpp"

#include "Includes/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"

VulkanCore::VQueueFamilyIndices VulkanCore::FindQueueFamilies(const vk::PhysicalDevice &physicalDevice) {
    uint32_t queueFamilyCount = 0;
    vk::QueueFamilyProperties queueFamilyPorperties;
    physicalDevice.getQueueFamilyProperties(&queueFamilyCount, &queueFamilyPorperties);
}

VulkanCore::VDevice::VDevice(const vk::Instance &instance):m_instance(instance) {
    m_physicalDevice = PickPhysicalDevice();
    m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice);
}

vk::PhysicalDevice VulkanCore::VDevice::PickPhysicalDevice() {
    auto availablePhysicalDevices = m_instance.enumeratePhysicalDevices();
    for(auto  physicalDevice: availablePhysicalDevices) {
        Utils::Logger::LogInfo("Found physical device: " + std::string(physicalDevice.getProperties().deviceName));
        if(GlobalVariables::GlobalStructs::primaryDeviceFeatures.CheckAgainstRetrievedPhysicalDevice(physicalDevice.getProperties().deviceType, physicalDevice.getFeatures()) ||
           GlobalVariables::GlobalStructs::secondaryDeviceFeatures.CheckAgainstRetrievedPhysicalDevice(physicalDevice.getProperties().deviceType, physicalDevice.getFeatures())) {

            Utils::Logger::LogSuccess("Going to use: " + std::string(physicalDevice.getProperties().deviceName) + " device, terminating next searches");
            return physicalDevice;
        }
    }
    throw std::runtime_error("Could not find a valid physical device try to disable some features");
}
