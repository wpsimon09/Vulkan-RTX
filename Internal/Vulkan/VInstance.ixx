//
// Created by wpsimon09 on 18/09/24.
//

module;
#include <vulkan/vulkan.h>
#include <iostream>

import Window;
export module VInstance;

export class VulkanInstance
{
    public:
        VulkanInstance();
        const VkInstance& GetInstance();
        ~VulkanInstance();
    private:
        VkInstance* m_vulkanInstance;
};

VulkanInstance::VulkanInstance()
{
    VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "Vulkan-RTX";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    const char** glfwExtenstions;
    uint32_t glfwExtensionCount;
    WindowManager::GetRequiredExtensions(glfwExtenstions, glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtenstions.
    createInfo.pNext = nullptr;

}

VulkanInstance::~VulkanInstance()
{
}
