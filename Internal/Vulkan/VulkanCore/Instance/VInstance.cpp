//
// Created by wpsimon09 on 22/09/24.
//

#include "VInstance.hpp"
#include "Includes/Logger/Logger.hpp"
#include "Includes/WindowManager/WindowManager.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VChecker.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"


PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;


VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,VkDebugUtilsMessengerEXT *pMessenger) {
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator) {
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

VulkanCore::VulkanInstance::VulkanInstance(std::string appname) {

    //-------------------------------
    // CHECK VALIDATION LAYER SUPPORT
    //-------------------------------
    if (GlobalState::ValidationLayersEnabled &&
        VulkanUtils::CheckValidationLayerSupport()) {
        Utils::Logger::LogSuccess("Validation are going to be used !");
    }
    else {
        Utils::Logger::LogInfo("Validation layers are not going to be used");
    }

    //-------------------------------
    // APPLICATION INFO
    //-------------------------------
    vk::ApplicationInfo applicationInfo;
    applicationInfo.pApplicationName = appname.c_str();
    applicationInfo.apiVersion = VK_API_VERSION_1_3;
    applicationInfo.applicationVersion = 1;
    applicationInfo.engineVersion = 1;
    applicationInfo.pEngineName = "Vulkan.hpp";


    //----------------------------------
    // GET EXTENSIONS
    //----------------------------------
    uint32_t extensionCount = 0;
    const char **extensions;
    WindowManager::GetRequiredExtensions(extensions, extensionCount);
    std::vector<const char *> extensionVector(extensions, extensions + extensionCount);

    //-------------------------------
    // INSTANCE CREATION
    //-------------------------------
    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.pApplicationInfo = &applicationInfo;
    if (GlobalState::ValidationLayersEnabled) {
        extensionVector.insert(extensionVector.end(), GlobalVariables::enabledExtensions.begin(), GlobalVariables::enabledExtensions.end());

        instanceInfo.enabledLayerCount = static_cast<uint32_t>(GlobalVariables::validationLayers.size());
        instanceInfo.ppEnabledLayerNames = GlobalVariables::validationLayers.data();

        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensionVector.size());
        instanceInfo.ppEnabledExtensionNames = extensionVector.data();
    }else {
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensionVector.size());
        instanceInfo.ppEnabledExtensionNames = extensionVector.data();
    }

    try {
        m_instance = vk::createInstance(instanceInfo);
        Utils::Logger::LogInfoVerboseOnly("Vulkan instance created");
    }
    catch (vk::SystemError &err) {
        throw std::runtime_error(err.what());
    }

    //----------------------------------------------------------
    // ADD CONFIGURE VALIDATION LAYERS ONCE INSTANCE IS CREATED
    //---------------------------------------------------------
    if(GlobalState::ValidationLayersEnabled) {
        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        if (!pfnVkCreateDebugUtilsMessengerEXT) {
            throw std::runtime_error("Failed to create debug messenger");
        }
        Utils::Logger::LogSuccess("Created create debug messenger");

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(m_instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

        if (!pfnVkDestroyDebugUtilsMessengerEXT) {
            throw std::runtime_error("Failed to create destroy debug messenger");
        }
        Utils::Logger::LogSuccess("Created destroy debug messenger");


        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

        m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(
            vk::DebugUtilsMessengerCreateInfoEXT({}, severityFlags, messageTypeFlags, &debugMessageFunc));
    }


}

VkBool32 VulkanCore::VulkanInstance::debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
  , VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData, void *) {
     std::ostringstream message;

    message << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": "
        << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ":\n";
    message << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
    message << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
    message << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
    if (0 < pCallbackData->queueLabelCount) {
        message << std::string("\t") << "Queue Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
            message << std::string("\t\t") << "LabelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount) {
        message << std::string("\t") << "CommandBuffer Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
            message << std::string("\t\t") << "LabelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->objectCount) {
        message << std::string("\t") << "Objects:\n";
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            message << std::string("\t\t") << "Object " << i << "\n";
            message << std::string("\t\t\t") << "objectType   = " << vk::to_string(
                static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
            message << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
            if (pCallbackData->pObjects[i].pObjectName) {
                message << std::string("\t\t\t") << "ObjectName   = <" << pCallbackData->pObjects[i].pObjectName <<
                    ">\n";
            }
        }
        std::string file = "Error in the file: \t";
        file += __FILE__;
        file += "\n";

        Utils::Logger::LogError(file + message.str());

        return false;
    }

}


VulkanCore::VulkanInstance::~VulkanInstance() {
    Utils::Logger::LogInfoVerboseOnly("Vulkan instance destroyed");
    if(GlobalState::ValidationLayersEnabled) {
        m_instance.destroyDebugUtilsMessengerEXT( m_debugMessenger );
    }
    m_instance.destroy();
}