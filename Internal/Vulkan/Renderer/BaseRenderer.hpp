//
// Created by wpsimon09 on 23/12/24.
//

#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanCore
{
    class VGraphicsPipeline;
}

namespace VulkanStructs
{
    struct RenderContext;
}

namespace VulkanCore
{
    class VDevice;
}

namespace Renderer
{
    class RenderTarget;

    class BaseRenderer
    {
    public:
        explicit BaseRenderer(const VulkanCore::VDevice& device);

    protected:
        virtual void RecordCommandBuffer(const VulkanCore::VGraphicsPipeline& pipeline) = 0;
        virtual void Render(const VulkanStructs::RenderContext& renderContext,const VulkanCore::VGraphicsPipeline& pipeline) = 0;
    protected:
        std::vector<std::unique_ptr<Renderer::RenderTarget>> m_renderTargets; // render to these images, per frame in flight
        const VulkanCore::VDevice& m_device;
        VulkanCore::VSyncPrimitive<vk::Semaphore> m_rendererFinishedSemaphore;
    };
} // Renderer

#endif //BASERENDERER_HPP
