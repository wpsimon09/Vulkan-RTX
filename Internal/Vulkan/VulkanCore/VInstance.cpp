//
// Created by wpsimon09 on 22/09/24.
//

#include "VInstance.hpp"
#include "Vulkan/Utils/VLogger.hpp"

VulkanCore::VulkanInstance::VulkanInstance(std::string appname)
{
    vk::ApplicationInfo applicationInfo;
    applicationInfo.pApplicationName = appname.c_str();
    applicationInfo.apiVersion = VK_API_VERSION_1_3;
    applicationInfo.applicationVersion = 1;
    applicationInfo.engineVersion = 1;
    applicationInfo.pEngineName = "Vulkan.hpp";

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.pApplicationInfo = &applicationInfo;

    try{
        m_instance = vk::createInstance(instanceInfo);
        Utils::VLogger::LogSuccess("Vulkan instance created");
    }catch (vk::SystemError& err){
        throw std::runtime_error(err.what());
    }
}

VulkanCore::VulkanInstance::~VulkanInstance()
{
    m_instance.destroy();
}
