//
// Created by wpsimon09 on 20/09/24.
//
module;
#include <vector>
#include <vulkan/vulkan.h>
#include <iostream>
import Logger;
export module VChecker;

namespace VChecker
{
    bool CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char* layerName : availableLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
            {
                std::string layerName = layerProperties.layerName;
            }
        }
    }
}
