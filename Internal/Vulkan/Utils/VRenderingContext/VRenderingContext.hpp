//
// Created by wpsimon09 on 23/03/25.
//

#ifndef VRENDERINGCONTEXT_HPP
#define VRENDERINGCONTEXT_HPP
#include <memory>
#include <optional>
#include <vector>
#include <glm/mat4x4.hpp>
#include "Application/Rendering/Scene/SceneData.hpp"
#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"

namespace Renderer {
class RenderTarget2;
}
namespace VulkanCore {
class VImage2;
}

namespace ApplicationCore {
class SkyBoxMaterial;
}


namespace VulkanUtils {

struct RenderContext
{
    glm::mat4 view{};
    glm::mat4 projection{};

    bool  RenderBillboards   = true;
    bool  RenderAABB         = false;
    bool  WireFrameRendering = false;
    float deltaTime          = 0.0;

    // other flags
    std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>> drawCalls;

    //===========================================
    // global illumination textures
    VulkanCore::VImage2* hdrCubeMap    = nullptr;
    VulkanCore::VImage2* irradianceMap = nullptr;
    VulkanCore::VImage2* prefilterMap  = nullptr;
    VulkanCore::VImage2* brdfMap       = nullptr;
    VulkanCore::VImage2* dummyCubeMap  = nullptr;

    //===========================================
    // atmosphere rendering textures
    VulkanCore::VImage2* transmitanceLut = nullptr;


    //===========================================
    // outputs from different render passes
    VulkanCore::VImage2*                 lightPassOutput  = nullptr;
    VulkanCore::VImage2*                 normalMap        = nullptr;
    VulkanCore::VImage2*                 positionMap      = nullptr;
    VulkanCore::VImage2*                 visibilityBuffer = nullptr;
    Renderer::RenderTarget2*             depthBuffer      = nullptr;
    std::shared_ptr<VulkanCore::VImage2> defaultTexture   = nullptr;

    std::shared_ptr<ApplicationCore::SkyBoxMaterial> SkyBox = nullptr;

    vk::AccelerationStructureKHR tlas;

    bool hasSceneChanged = false;

    //===============================================================
    // DRAW CALLS SECTION
    std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& GetAllDrawCall();

    ApplicationCore::SceneData renderingSceneData;  // contains not the entire scene but only what is visible in the view port

    std::optional<VulkanStructs::VDrawCallData> fogDrawCall;
    std::optional<VulkanStructs::VDrawCallData> atmosphereCall;

    void GetAllDrawCall(std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& outDrawCalls);
    void AddDrawCall(VulkanStructs::VDrawCallData& DrawCall);
    void ResetAllDrawCalls();

    static unsigned long GenerateDrawKey(VulkanStructs::VDrawCallData& drawCall);
};
}  // namespace VulkanUtils


#endif  //VRENDERINGCONTEXT_HPP
