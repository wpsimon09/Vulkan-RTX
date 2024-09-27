//
// Created by wpsimon09 on 25/09/24.
//

#include "VDevice.hpp"

#include <set>

#include "Includes/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

VulkanCore::VQueueFamilyIndices VulkanCore::FindQueueFamilies(const vk::PhysicalDevice &physicalDevice) {
    VulkanCore::VQueueFamilyIndices indices;

    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();;

    indices.graphicsFamily = VulkanUtils::FindQueueFamily(queueFamilyProperties, vk::QueueFlagBits::eGraphics);
    return indices;
}

VulkanCore::VDevice::VDevice(const vk::Instance &instance):m_instance(instance) {
    m_physicalDevice = PickPhysicalDevice();
    CreateLogicalDevice();
}

VulkanCore::VDevice::~VDevice() {
    m_device.destroy();
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

void VulkanCore::VDevice::CreateLogicalDevice() {
    m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily.value()};

    // we have to tell logical device what queue families it will have
    float queuePriority = 1.0f;
    for(auto queueFamily: uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // now we have to tell logical device what physical device features we will be using
    // TODO: this will grow in future
    vk::PhysicalDeviceFeatures deviceFeatures{};

    //create the logical device
    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    m_device = m_physicalDevice.createDevice(deviceCreateInfo);
    if(!m_device) {
        throw std::runtime_error("Could not create logical device");
    }else {
        Utils::Logger::LogSuccess("Successfully created logical device");
    }
    m_graphicsQueue = m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value(), 0);
    if(!m_graphicsQueue) {
        throw std::runtime_error("Could not retrieve graphics queue");
    }else {
        Utils::Logger::LogSuccess("Successfully retrieved graphics queue");
    }
}
