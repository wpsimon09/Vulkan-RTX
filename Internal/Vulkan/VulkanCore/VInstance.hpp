//
// Created by wpsimon09 on 22/09/24.
//

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

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            vk::DebugUtilsMessageTypeFlagBitsEXT messageSeverity,
            vk::DebugUtilsMessageTypeFlagsEXT messageType,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void *pUserData
            );

        ~VulkanInstance();

    private:
        void PopulateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
    };
}

#endif //VINSTANCE_HPP
