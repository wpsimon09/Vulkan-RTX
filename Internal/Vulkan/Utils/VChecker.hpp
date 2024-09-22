//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VCHECKER_HPP
#define VCHECKER_HPP

#include <vulkan/vulkan.hpp>
#include "Includes/Logger/Logger.hpp"

namespace VulkanUtils
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    export bool CheckValidationLayerSupport() {
        uint32_t layerCount;

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
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        return true;
    }
};


#endif //VCHECKER_HPP
