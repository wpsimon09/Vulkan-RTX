//
// Created by wpsimon09 on 18/09/24.
//

module;
#include <vulkan/vulkan.h>
#include <iostream>

import VSelector;
export module VInstance;

export class VulkanInstance
{
    public:
        VulkanInstance();
        const VkInstance& GetInstance() const;
        ~VulkanInstance();
    private:
        VkInstance m_vulkanInstance;
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
    auto extensions = VulkanSelector::GetRequiredExtensions();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_vulkanInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create intance \n");
    }
    else
    {
        std::cout << "Vulkan instance created successfuly \n";
    }
}

const VkInstance& VulkanInstance::GetInstance() const
{
    return m_vulkanInstance;
}

VulkanInstance::~VulkanInstance()
{
    vkDestroyInstance(m_vulkanInstance, nullptr);
}
