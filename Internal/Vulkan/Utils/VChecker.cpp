//
// Created by wpsimon09 on 22/09/24.
//

#include "VChecker.hpp"
#include <vulkan/vulkan.hpp>
#include "Includes/Logger/Logger.hpp"

bool VulkanUtils::  CheckValidationLayerSupport()
{
    uint32_t layerCount;
    Utils::Logger::LogInfo("Started to look for the validation layers !");

    if (vk::enumerateInstanceLayerProperties(&layerCount, nullptr) != vk::Result::eSuccess) {
        Utils::Logger::LogError("vk::enumerateInstanceLayerProperties failed");
        return false;
    }

    std::vector<vk::LayerProperties> availableLayers(layerCount);
    if (vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != vk::Result::eSuccess) {
        Utils::Logger::LogError("vk::enumerateInstanceLayerProperties failed");
        return false;
    }

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            std::string layerNamePrint = layerProperties.layerName;
            Utils::Logger::LogInfo("Found layer:" + layerNamePrint);
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                Utils::Logger::LogSuccess("Found validation layer, stopping the search");
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

bool VulkanUtils::CheckPhysicalDeviceCompatibility(vk::PhysicalDevice physicalDevice) {
    auto properties = physicalDevice.getProperties();
    auto features = physicalDevice.getFeatures();


}