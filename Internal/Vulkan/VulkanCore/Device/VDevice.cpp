//
// Created by wpsimon09 on 25/09/24.
//

#include "VDevice.hpp"

#include "Includes/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"

VulkanCore::VQueueFamilyIndices VulkanCore::FindQueueFamilies(const vk::PhysicalDevice &physicalDevice) {
    VulkanCore::VQueueFamilyIndices indices;

    //get all queue families on the device
    const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    //select just the queue fmily index that supports graphics operations
    std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty = std::find_if(
        queueFamilyProperties.begin(),
        queueFamilyProperties.end(),
        []( vk::QueueFamilyProperties const & qfp ) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; } );

    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    indices.graphicsFamily = static_cast<uint32_t> (std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    Utils::Logger::LogInfoVerboseOnly("Found graphics queue family at index" + indices.graphicsFamily.value());

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
