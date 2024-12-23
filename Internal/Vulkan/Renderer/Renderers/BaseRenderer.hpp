//
// Created by wpsimon09 on 23/12/24.
//

#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanCore
{
    class VSwapChain;
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
        virtual void CreateRenderTargets(std::optional<VulkanCore::VSwapChain&> swapChain = std::nullopt) {};
        virtual void RecordCommandBuffer(const VulkanCore::VGraphicsPipeline& pipeline) = 0;
        virtual void Render(const VulkanStructs::RenderContext& renderContext,const VulkanCore::VGraphicsPipeline& pipeline) = 0;

        const VulkanCore::VRenderPass& GetRenderPass(int currentFrame ) const {return *m_renderTargets[currentFrame]->m_renderPass;};
        const VulkanCore::VImage& GetRenderedImage(int currentFrame ) const {return *m_renderTargets[currentFrame]->m_colourBuffer[currentFrame];}; // i have to place fence to access the imag
        const VulkanCore::VFrameBuffer& GetFrameBuffer(int currentFrame ) const {return *m_renderTargets[currentFrame]->m_frameBuffers[currentFrame];};
    protected:
        std::vector<std::unique_ptr<Renderer::RenderTarget>> m_renderTargets; // render to these images, per frame in flight
        const VulkanCore::VDevice& m_device;
        VulkanCore::VSyncPrimitive<vk::Semaphore> m_rendererFinishedSemaphore;

        int m_width, m_height;
    };
} // Renderer

#endif //BASERENDERER_HPP
