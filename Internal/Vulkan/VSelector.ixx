//
// Created by wpsimon09 on 20/09/24.
//

module;

#include <vector>
#include <iostream>

export module VSelector;
import Window;
import GlobalState;
import vulkan_hpp;

export namespace VulkanSelector
{
     std::vector<const char*> GetRequiredExtensions()
     {
          u_int32_t glfwExtentionsCount = 0;
          const char** glfwExtentions;

          WindowManager::GetRequiredExtensions(glfwExtentions, glfwExtentionsCount);
          std::vector<const char*> extensions(glfwExtentions, glfwExtentions + glfwExtentionsCount);

          if (GlobalState::ENABLE_VALIDATION_LAYERS)
          {
               extensions.push_back(vk::EXTDebugUtilsExtensionName);
          }

          return extensions;
     }
}
