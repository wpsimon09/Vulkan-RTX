//
// Created by wpsimon09 on 20/09/24.
//

module;

#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

export module VSelector;
import Window;
import GlobalState;

export namespace VulkanSelector
{
     std::vector<const char*> GetRequiredExtensions()
     {
          uint32_t glfwExtentionsCount = 0;
          const char** glfwExtentions;

          WindowManager::GetRequiredExtensions(glfwExtentions, glfwExtentionsCount);
          std::vector<const char*> extensions(glfwExtentions, glfwExtentions + glfwExtentionsCount);

          if (GlobalState::ENABLE_VALIDATION_LAYERS)
          {
               extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
          }

          return extensions;
     }
}
