//
// Created by wpsimon09 on 21/12/24.
//

#ifndef RENDERINGSYSTEM_HPP
#define RENDERINGSYSTEM_HPP
#include <memory>
#include <vector>
#include <glm/fwd.hpp>

#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace Renderer
{
    class UserInterfaceRenderer;
}

namespace Renderer
{
    class SceneRenderer;
}

namespace VulkanStructs
{
    struct RenderContext;
}

namespace VulkanCore
{
    class VDevice;
}

namespace VulkanUtils
{
    class VUniformBufferManager;
    class VPushDescriptorManager;
}

namespace Renderer {

class RenderingSystem {
public:
private:
    const VulkanCore::VDevice &m_device;
    const VulkanUtils::VUniformBufferManager &m_uniformBufferManager;
    VulkanUtils::VPushDescriptorManager &m_pushDescriptorSetManager;
    uint32_t m_currentImageIndex = 0;
    glm::uint32_t m_currentFrameIndex = 0;

    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_imageAvailableSemaphores;
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_renderFinishedSemaphores;
    std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Fence>>> m_isFrameFinishFences;

    std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;

    VulkanStructs::RenderContext* m_renderingContext;;

    std::unique_ptr<Renderer::SceneRenderer> m_sceneRenderer;
    std::unique_ptr<Renderer::UserInterfaceRenderer> m_uiRenderer;



};

} // Renderer

#endif //RENDERINGSYSTEM_HPP
