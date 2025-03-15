//
// Created by wpsimon09 on 21/12/24.
//

#ifndef RENDERINGSYSTEM_HPP
#define RENDERINGSYSTEM_HPP
#include <memory>
#include <vector>
#include <glm/fwd.hpp>

#include "Editor/Views/UserInterface/IUserInterfaceElement.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanCore
{
    class VTimelineSemaphore;
    class VPipelineManager;
    class VSwapChain;
    class VDevice;
}

namespace VEditor
{
    class RenderingOptions;
    class UIContext;
}

namespace Renderer
{
    class UserInterfaceRenderer;
    class SceneRenderer;
}

namespace VulkanStructs
{
    struct RenderContext;
}

namespace VulkanUtils
{
    class UIContext;
    class VUniformBufferManager;
    class VPushDescriptorManager;
}

struct GlobalUniform;

namespace Renderer {

class RenderingSystem {
public:
    RenderingSystem(const VulkanCore::VulkanInstance& instance,
                    const VulkanCore::VDevice& device,
                    const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                    VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                    VEditor::UIContext &uiContext);

    VulkanStructs::RenderContext* GetRenderContext() {return &m_renderContext;}
public:
    void Init();
    void Render(LightStructs::SceneLightInfo& sceneLightInfo,GlobalUniform& globalUniformUpdateInfo);
    void Update();
    void Destroy();

private:

    const VulkanCore::VDevice &m_device;
    const VulkanUtils::VUniformBufferManager &m_uniformBufferManager;
    VEditor::UIContext &m_uiContext;

    VulkanUtils::VPushDescriptorManager &m_pushDescriptorSetManager;
    uint32_t m_currentImageIndex = 0;
    uint32_t m_currentFrameIndex = 0;

    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_imageAvailableSemaphores;

    std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;

    VulkanStructs::RenderContext m_renderContext;

    std::unique_ptr<Renderer::SceneRenderer> m_sceneRenderer;
    std::unique_ptr<Renderer::UserInterfaceRenderer> m_uiRenderer;
    std::unique_ptr<VulkanCore::VPipelineManager> m_pipelineManager;
    std::vector<std::unique_ptr<VulkanCore::VTimelineSemaphore>> m_renderingTimeLine;
    VulkanCore::VTimelineSemaphore &m_transferSemapohore;

    bool m_isRayTracing = false;

    friend class VEditor::RenderingOptions;
    friend class VEditor::UIContext;
};

} // Renderer

#endif //RENDERINGSYSTEM_HPP
