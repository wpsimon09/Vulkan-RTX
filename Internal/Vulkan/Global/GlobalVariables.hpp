//
// Created by wpsimon09 on 22/09/24.
//

#ifndef GLOBALVARIABLES_HPP
#define GLOBALVARIABLES_HPP

#include <vector>

#include "Application/Structs/ApplicationStructs.hpp"
#include "filesystem"


namespace VEditor {
class UIContext;
}

class Client;

namespace GlobalVariables {
inline const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

inline const std::vector<const char*> instanceLevelExtensions = {
    static_cast<const char*>(VK_EXT_DEBUG_UTILS_EXTENSION_NAME),

};

inline const std::vector<const char*> deviceLevelExtensions = {
    static_cast<const char*>(VK_KHR_SWAPCHAIN_EXTENSION_NAME),
    static_cast<const char*>(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME),
    static_cast<const char*>(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME),
    static_cast<const char*>(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME),
    static_cast<const char*>(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME),
    static_cast<const char*>(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME),

};

enum SHADER_TYPE
{
  FRAGMENT = 0,
  VERTEX   = 1,
  COMPUTE  = 2,
};

inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

inline int ID_COUNTER = 0;

inline constexpr int RenderTargetResolutionWidth  = 1920;
inline constexpr int RenderTargetResolutionHeight = 1080;

inline constexpr int MAX_THREADS = 14;

inline int PointLightCount = 0;

inline const std::filesystem::path configFolder("cache/");
inline const std::filesystem::path textureFolder("cache/textures/");
inline const std::filesystem::path defaultTextureFile("Resources/DefaultTexture.jpg");
inline std::filesystem::path       lightInfoPath("cache/Light.ini");

inline bool hasSessionBeenSaved = false;


}  // namespace GlobalVariables

#endif  //GLOBALVARIABLES_HPP