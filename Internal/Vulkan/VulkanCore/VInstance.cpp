//
// Created by wpsimon09 on 22/09/24.
//

#include "VInstance.hpp"
#include "Includes/Logger/Logger.hpp"
#include "Includes/WindowManager/WindowManager.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VChecker.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"


VkBool32 VulkanCore::VulkanInstance::debugCallback(vk::DebugUtilsMessageTypeFlagBitsEXT messageSeverity,
                                                   vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                                   const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                   void *pUserData) {
    std::ostringstream message;

    message << vk::to_string( static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>( messageSeverity ) ) << ": "
            << vk::to_string( static_cast<vk::DebugUtilsMessageTypeFlagsEXT>( messageType ) ) << ":\n";
    message << std::string( "\t" ) << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
    message << std::string( "\t" ) << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
    message << std::string( "\t" ) << "message         = <" << pCallbackData->pMessage << ">\n";
    if ( 0 < pCallbackData->queueLabelCount )
    {
        message << std::string( "\t" ) << "Queue Labels:\n";
        for ( uint32_t i = 0; i < pCallbackData->queueLabelCount; i++ )
        {
            message << std::string( "\t\t" ) << "LabelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
    }
    if ( 0 < pCallbackData->cmdBufLabelCount )
    {
        message << std::string( "\t" ) << "CommandBuffer Labels:\n";
        for ( uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++ )
        {
            message << std::string( "\t\t" ) << "LabelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
    }
    if ( 0 < pCallbackData->objectCount )
    {
        message << std::string( "\t" ) << "Objects:\n";
        for ( uint32_t i = 0; i < pCallbackData->objectCount; i++ )
        {
            message << std::string( "\t\t" ) << "Object " << i << "\n";
            message << std::string( "\t\t\t" ) << "objectType   = " << vk::to_string( static_cast<vk::ObjectType>( pCallbackData->pObjects[i].objectType ) ) << "\n";
            message << std::string( "\t\t\t" ) << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
            if ( pCallbackData->pObjects[i].pObjectName )
            {
                message << std::string( "\t\t\t" ) << "ObjectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
            }
        }
    }
    std::string file = "Error in the file: \t";
    file += __FILE__;
    file += "\n";

    Utils::Logger::LogError(file + message.str() );

    return false;
}

VulkanCore::VulkanInstance::VulkanInstance(std::string appname) {
    if (GlobalState::ValidationLayersEnabled &&
        VulkanUtils::CheckValidationLayerSupport()) {
        Utils::Logger::LogSuccess("Validation layers were found");
    }
    else {
        throw std::runtime_error("Failed to find validation layers");
    }

    vk::ApplicationInfo applicationInfo;
    applicationInfo.pApplicationName = appname.c_str();
    applicationInfo.apiVersion = VK_API_VERSION_1_3;
    applicationInfo.applicationVersion = 1;
    applicationInfo.engineVersion = 1;
    applicationInfo.pEngineName = "Vulkan.hpp";

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.pApplicationInfo = &applicationInfo;

    uint32_t extensionCount = 0;
    const char** extensions;

    WindowManager::GetRequiredExtensions(extensions, extensionCount);

    std::vector<const char*> extensionVector(extensions, extensions + extensionCount);
    extensionVector.insert(extensionVector.end(), GlobalVariables::validationLayers.begin(), GlobalVariables::validationLayers.end());

    if (GlobalState::ValidationLayersEnabled) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(GlobalVariables::validationLayers.size());
        instanceInfo.ppEnabledLayerNames = GlobalVariables::validationLayers.data();
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensionVector.size());
        instanceInfo.ppEnabledExtensionNames = GlobalVariables::enabledExtensions.data();
    }

    try {
        m_instance = vk::createInstance(instanceInfo);
        Utils::Logger::LogInfoVerboseOnly("Vulkan instance created");
    }
    catch (vk::SystemError &err) {
        throw std::runtime_error(err.what());
    }

}

VulkanCore::VulkanInstance::~VulkanInstance() {
    Utils::Logger::LogSuccess("Vulkan instance destroyed");
    m_instance.destroy();
}

void VulkanCore::VulkanInstance::PopulateDebugMessengerCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags( vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError );
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags( vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation );


}
