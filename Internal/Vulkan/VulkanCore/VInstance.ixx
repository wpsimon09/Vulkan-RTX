//
// Created by wpsimon09 on 18/09/24.
//

module;

#include <vulkan/vulkan.hpp>

import VLogger;

export module VInstance;

export namespace VulkanCore
{
    class VulkanInstance
    {
        public:
            explicit VulkanInstance(std::string appname);
            ~VulkanInstance();
        private:
            vk::Instance m_instance;
    };

    VulkanInstance::VulkanInstance(std::string appname)
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
            VLogger::LogSuccess("Vulkan instance created");
        }catch (vk::SystemError& err){
            throw std::runtime_error(err.what());
        }
    }

    VulkanInstance::~VulkanInstance()
    {
        m_instance.destroy();
    }
}





