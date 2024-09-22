//
// Created by wpsimon09 on 20/09/24.
//
module;

#include <vector>
#include <iostream>
#include <cstring>
#include <vulkan/vulkan.hpp>

import Logger;

export module VChecker;

namespace VChecker
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    export bool CheckValidationLayerSupport()
    {
        u_int32_t layerCount;

        if(vk::enumerateInstanceLayerProperties(&layerCount, nullptr) != vk::Result::eSuccess)
        {
            Logger::LogError("vk::enumerateInstanceLayerProperties failed");
            return false;
        }


        std::vector<vk::LayerProperties> availableLayers(layerCount);
        if(vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != vk::Result::eSuccess)
        {
            Logger::LogError("vk::enumerateInstanceLayerProperties failed");
            return false;
        }

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
