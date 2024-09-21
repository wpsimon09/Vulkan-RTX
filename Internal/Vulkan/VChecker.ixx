//
// Created by wpsimon09 on 20/09/24.
//
module;
#include <vector>
#include <iostream>
#include <cstring>
import Logger;
export module VChecker;
import vulkan_hpp;

namespace VChecker
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    export bool CheckValidationLayerSupport()
    {
        u_int32_t layerCount;
        vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<vk::LayerProperties> availableLayers(layerCount);
        vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
            {
                std::string layerNamePrint = layerProperties.layerName;
                Logger::LogInfo("Found layer:" + layerNamePrint);
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
