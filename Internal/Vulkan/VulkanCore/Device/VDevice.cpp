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
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"

VulkanCore::VQueueFamilyIndices VulkanCore::FindQueueFamilies(const vk::PhysicalDevice &physicalDevice,const VulkanCore::VulkanInstance& instance) {
    VulkanCore::VQueueFamilyIndices indices;

    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();;

    //------------------------
    // NORMAL QUEUE FAMILIES
    //------------------------
    indices.graphicsFamily = VulkanUtils::FindQueueFamily(queueFamilyProperties, vk::QueueFlagBits::eGraphics);

    //----------------------
    // TRANSFER QUEUE FAMILY
    //----------------------
    vk::Bool32 presentSupport = false;
    for(int i = 0; i<queueFamilyProperties.size(); i++) {
        presentSupport = physicalDevice.getSurfaceSupportKHR(i, instance.GetSurface());
        indices.presentFamily = i;
        break;
    }
    assert(presentSupport == true);
    Utils::Logger::LogInfoVerboseOnly("Found transfer queue family at index: " + std::to_string(indices.presentFamily.value()));


    return indices;
}

VulkanCore::VDevice::VDevice(const VulkanCore::VulkanInstance& instance):m_instance(instance) {
    m_physicalDevice = PickPhysicalDevice();
    CreateLogicalDevice();
}

vk::PhysicalDevice VulkanCore::VDevice::PickPhysicalDevice() {
    auto availablePhysicalDevices = m_instance.GetInstance().enumeratePhysicalDevices();
    for(auto  physicalDevice: availablePhysicalDevices) {
        Utils::Logger::LogInfo("Found physical device: " + std::string(physicalDevice.getProperties().deviceName));
        if(GlobalVariables::GlobalStructs::primaryDeviceFeatures.CheckAgainstRetrievedPhysicalDevice(physicalDevice, m_instance.GetSurface()) ||
           GlobalVariables::GlobalStructs::secondaryDeviceFeatures.CheckAgainstRetrievedPhysicalDevice(physicalDevice, m_instance.GetSurface())) {

            Utils::Logger::LogSuccess("Going to use: " + std::string(physicalDevice.getProperties().deviceName) + " device, terminating next searches");
            return physicalDevice;
        }
    }
    throw std::runtime_error("Could not find a valid physical device try to disable some features");
}

void VulkanCore::VDevice::CreateLogicalDevice() {
    m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_instance);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value()};

    // we have to tell logical device what queue families it will have
    float queuePriority = 1.0f;
    for(auto queueFamily: uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    //create the logical device
    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.ppEnabledExtensionNames = GlobalVariables::deviceLevelExtensions.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(GlobalVariables::deviceLevelExtensions.size());

    if(GlobalState::ValidationLayersEnabled) {
        deviceCreateInfo.enabledLayerCount = GlobalVariables::validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = GlobalVariables::validationLayers.data();
    }else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    m_device = m_physicalDevice.createDevice(deviceCreateInfo);
    assert(m_device);
    Utils::Logger::LogSuccess("Successfully created logical device");


    m_graphicsQueue = m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value(), 0);
    assert(m_graphicsQueue != VK_NULL_HANDLE);
    Utils::Logger::LogSuccess("Successfully retrieved graphics queue");


    m_presentQueue = m_device.getQueue(m_queueFamilyIndices.presentFamily.value(), 0);if(!m_presentQueue)
    assert(m_presentQueue != VK_NULL_HANDLE);Utils::Logger::LogSuccess("Successfully retrieved present queue");

}

void VulkanCore::VDevice::Destroy() {
    m_device.destroy();
}
