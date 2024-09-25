//
// Created by wpsimon09 on 22/09/24.
//
#pragma once

#ifndef VINSTANCE_HPP
#define VINSTANCE_HPP

#include <string>
#include <vulkan/vulkan.hpp>


namespace VulkanCore
{
    class VulkanInstance
    {
    public:
        explicit VulkanInstance(std::string appname);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc( VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT              messageTypes,
                                                 VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData,
                                                 void * /*pUserData*/ );

        const vk::Instance& GetInstance() const { return m_instance; }

        ~VulkanInstance();
    private:
        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
    };
}

#endif //VINSTANCE_HPP
