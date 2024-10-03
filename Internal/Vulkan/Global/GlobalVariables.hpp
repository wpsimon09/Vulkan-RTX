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

    const std::vector<const char *> instanceLevelExtensions = {
      static_cast<const char *>(VK_EXT_DEBUG_UTILS_EXTENSION_NAME),
    };

    const std::vector<const char *> deviceLevelExtensions = {
      static_cast<const char *>(VK_KHR_SWAPCHAIN_EXTENSION_NAME),
    };

    enum SHADER_TYPE
    {
        FRAGMENT = 0,
        VERTEX = 1,
        COMPUTE = 2,
    };


}

#endif //GLOBALVARIABLES_HPP