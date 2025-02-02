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

namespace VEditor
{
    class RenderingOptions;
}

namespace VEditor
{
    class UIContext;
}

struct GlobalUniform;

namespace Renderer
{
    class UserInterfaceRenderer;
    class SceneRenderer;
}

namespace Renderer
{
}

namespace VulkanStructs
{
    struct RenderContext;
}

namespace VulkanCore
{
    class VPipelineManager;
    class VSwapChain;
    class VDevice;
}

namespace VulkanUtils
{
    class UIContext;
    class VUniformBufferManager;
    class VPushDescriptorManager;
}

namespace Renderer {

class RenderingSystem {
public:
    RenderingSystem(const VulkanCore::VulkanInstance& instance,
                    const VulkanCore::VDevice& device,
                    const VulkanUtils::VUniformBufferManager& uniformBufferManager,
                    VulkanUtils::VPushDescriptorManager& pushDescriptorManager,
                    VEditor::UIContext &uiContext);

    VulkanStructs::RenderContext* GetRenderContext() {return &m_mainRenderContext;}
public:
    void Init();
    void Render(GlobalUniform& globalUniformUpdateInfo);
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
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_renderFinishedSemaphores;
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Fence>>> m_isFrameFinishFences;

    std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;

    VulkanStructs::RenderContext m_mainRenderContext;

    std::unique_ptr<Renderer::SceneRenderer> m_sceneRenderer;
    std::unique_ptr<Renderer::UserInterfaceRenderer> m_uiRenderer;
    std::unique_ptr<VulkanCore::VPipelineManager> m_pipelineManager;

    bool m_isRayTracing = false;
    bool m_allowEditorBillboards = true;

    friend

    class VEditor::RenderingOptions;

};

} // Renderer

#endif //RENDERINGSYSTEM_HPP
