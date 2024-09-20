//
// Created by wpsimon09 on 20/09/24.
//
module;
#include <vector>
#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>
import Logger;
export module VChecker;

namespace VChecker
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    export bool CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
            {
                std::string layerNamePrint = layerProperties.layerName;
                Logger::LogInfo("Found layer" + layerNamePrint);
                if(strcmp(layerName,layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }
            if(!layerFound)
            {
                return false;
            }
        }
        return true;
    }
}
