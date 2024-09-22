//
// Created by wpsimon09 on 22/09/24.
//

#include "VInstance.hpp"
#include "Includes/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/VChecker.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"

VulkanCore::VulkanInstance::VulkanInstance(std::string appname) {
  if ( GlobalState::ValidationLayersEnabled &&
       VulkanUtils::CheckValidationLayerSupport() ) {
    Utils::Logger::LogSuccess("Validation layers were found");
  }
  else {
    throw std::runtime_error("Failed to find validation layers");
  }

  vk::ApplicationInfo applicationInfo;
  applicationInfo.pApplicationName   = appname.c_str();
  applicationInfo.apiVersion         = VK_API_VERSION_1_3;
  applicationInfo.applicationVersion = 1;
  applicationInfo.engineVersion      = 1;
  applicationInfo.pEngineName        = "Vulkan.hpp";

  vk::InstanceCreateInfo instanceInfo;
  instanceInfo.pApplicationInfo = &applicationInfo;

  if ( GlobalState::ValidationLayersEnabled ) {
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(GlobalVariables::validationLayers.size());
    instanceInfo.ppEnabledLayerNames  = GlobalVariables::validationLayers.data();
  }

  try {
    m_instance = vk::createInstance(instanceInfo);
    Utils::Logger::LogInfoVerboseOnly("Vulkan instance created");
  } catch ( vk::SystemError &err ) {
    throw std::runtime_error(err.what());
  }
}

VulkanCore::VulkanInstance::~VulkanInstance() {
  Utils::Logger::LogSuccess("Vulkan instance destroyed");
  m_instance.destroy();
}

void VulkanCore::VulkanInstance::PopulateDebugMessengerCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
    vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;
}