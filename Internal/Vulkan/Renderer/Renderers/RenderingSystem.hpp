//
// Created by wpsimon09 on 21/12/24.
//

#ifndef RENDERINGSYSTEM_HPP
#define RENDERINGSYSTEM_HPP

// STL
#include <memory>
#include <vector>

// GLM forward declaration
#include <glm/fwd.hpp>

// Project Includes
#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanCore {
class VDescriptorLayoutCache;
}
namespace ApplicationCore {
struct SceneData;
}
// Forward Declarations
namespace VulkanCore {
    class VDevice;
    class VSwapChain;
    class VPipelineManager;
    class VTimelineSemaphore;
    template<typename T>
    class VSyncPrimitive;
}

namespace VulkanUtils {
    class UIContext;
    class VEnvLightGenerator;
    class VUniformBufferManager;
    class VResourceGroupManager;
    class VRayTracingDataManager;
    struct RenderContext;
}

namespace VulkanCore::RTX {
    struct BLASInput;
}

namespace Renderer {
class PostProcessingSystem;
class RayTracer;
    class ForwardRenderer;
    class UserInterfaceRenderer;
}

namespace VEditor {
    class RenderingOptions;
    class UIContext;
}

struct GlobalUniform;

namespace Renderer {

class RenderingSystem
{
public:
    RenderingSystem(const VulkanCore::VulkanInstance&         instance,
                    const VulkanCore::VDevice&                device,
                    VulkanUtils::VRayTracingDataManager& m_rayTracingDataManager,
                     VulkanUtils::VUniformBufferManager& uniformBufferManager,
                    ApplicationCore::EffectsLibrary&          effectsLybrary,
                    VulkanCore::VDescriptorLayoutCache&       descLayoutCache,
                    VEditor::UIContext&                       uiContext);

    void Init();
    void Render(bool resizeSwapChain, LightStructs::SceneLightInfo& sceneLightInfo,ApplicationCore::SceneData& sceneData, GlobalUniform& globalUniformUpdateInfo, PostProcessingParameters& postProcessingParameters, SceneUpdateFlags& sceneUpdateFlags);
    void Update();
    void Destroy();

    ForwardRenderer&                         GetSceneRenderer() { return *m_forwardRenderer; };
    VulkanUtils::RenderContext*            GetRenderContext() { return &m_renderContext; }

private:
    // Core Vulkan references
    const VulkanCore::VDevice&                m_device;
     VulkanUtils::VUniformBufferManager& m_uniformBufferManager;
    ApplicationCore::EffectsLibrary*          m_effectsLibrary;
    VEditor::UIContext&                       m_uiContext;

    // Scene state
    LightStructs::SceneLightInfo* m_sceneLightInfo = nullptr;

    // Renderers
    std::unique_ptr<Renderer::ForwardRenderer>       m_forwardRenderer;
    std::unique_ptr<Renderer::PostProcessingSystem>  m_postProcessingSystem;
    std::unique_ptr<Renderer::UserInterfaceRenderer> m_uiRenderer;
    std::unique_ptr<Renderer::RayTracer>             m_rayTracer;

    // Environment
    std::unique_ptr<VulkanUtils::VEnvLightGenerator> m_envLightGenerator;

    // Swapchain and Command Buffers
    std::unique_ptr<VulkanCore::VSwapChain>                    m_swapChain;
    std::unique_ptr<VulkanCore::VCommandPool>                  m_renderingCommandPool;
    std::vector<std::unique_ptr<VulkanCore::VCommandBuffer>>   m_renderingCommandBuffers;

    // Synchronization
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_imageAvailableSemaphores;
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_ableToPresentSemaphore;
    std::vector<std::unique_ptr<VulkanCore::VTimelineSemaphore>>            m_renderingTimeLine;
    VulkanCore::VTimelineSemaphore&                                           m_transferSemapohore;


    // Render context
    VulkanUtils::RenderContext m_renderContext;
    VulkanStructs::PostProcessingContext m_postProcessingContext;

    // State
    uint32_t m_currentImageIndex = 0;
    uint32_t m_currentFrameIndex = 0;
    uint64_t m_frameCount = 0;
    uint64_t m_accumulatedFramesCount = 0;
    bool     m_isRayTracing = false;

    VulkanCore::VDescriptorLayoutCache& m_descLayoutCache;

    // Editor Integration
    friend class VEditor::RenderingOptions;
    friend class VEditor::UIContext;
};

}  // namespace Renderer

#endif  // RENDERINGSYSTEM_HPP
