//
// Created by wpsimon09 on 22/09/24.
//

#ifndef GLOBALVARIABLES_HPP
#define GLOBALVARIABLES_HPP

#include <vector>


namespace GlobalVariables {
    const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char *> enabledExtensions = {
      static_cast<const char *>(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
    };

    PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

}

#endif //GLOBALVARIABLES_HPP