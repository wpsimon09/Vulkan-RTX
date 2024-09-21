//
// Created by wpsimon09 on 18/09/24.
//

module;
#include <iostream>

import vulkan_hpp;
import VSelector;
import VChecker;

import Logger;
import GlobalState;

export module VInstance;

export class VulkanInstance
{
    public:
        VulkanInstance();
        const vk::Instance& GetInstance() const;
        ~VulkanInstance();
    private:
        vk::Instance m_vulkanInstance;
};

VulkanInstance::VulkanInstance()
{
    if(GlobalState::ENABLE_VALIDATION_LAYERS && !VChecker::CheckValidationLayerSupport())
    {
        throw std::runtime_error("Requested validation layers were not found");
    }
    else
    {
        Logger::LogSuccess("Valiation layers found") ;
    }

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Vulkan-RTX";
    appInfo.applicationVersion = vk::makeVersion(1, 0, 0);
    appInfo.pEngineName = "Vulkan.hpp";
    appInfo.engineVersion =  vk::makeVersion(1, 0, 0);
    appInfo.apiVersion = vk::makeApiVersion(1,3,0);
    appInfo.pNext = nullptr;

    vk::InstanceCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    auto extensions = VulkanSelector::GetRequiredExtensions();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    m_vulkanInstance = vk::createInstance(createInfo);
    Logger::LogSuccess("Vulkan instance created successfuly");
}

const vk::Instance& VulkanInstance::GetInstance() const
{
    return m_vulkanInstance;
}

VulkanInstance::~VulkanInstance()
{
    m_vulkanInstance.destroy();
}
