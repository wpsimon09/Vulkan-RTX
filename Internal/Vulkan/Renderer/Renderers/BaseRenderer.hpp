//
// Created by wpsimon09 on 23/12/24.
//

#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

struct GlobalUniform;

namespace VulkanCore
{
    class VCommandBuffer;
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
        virtual ~BaseRenderer() = default;

        const VulkanCore::VRenderPass& GetRenderPass(int currentFrame ) const {return *m_renderTargets[currentFrame]->m_renderPass;};
        const VulkanCore::VImage& GetRenderedImage(int currentFrame ) const {return *m_renderTargets[currentFrame]->m_colourBuffer[currentFrame];}; // i have to place fence to access the image
        const VulkanCore::VFrameBuffer& GetFrameBuffer(int currentFrame ) const {return *m_renderTargets[currentFrame]->m_frameBuffers[currentFrame];};

        virtual void Render(GlobalUniform& globalUniformUpdateInfo, const VulkanStructs::RenderContext& renderContext,const VulkanCore::VGraphicsPipeline& pipeline) = 0;
    protected:
        virtual void CreateRenderTargets(VulkanCore::VSwapChain* swapChain = nullptr) {};
        virtual void RecordCommandBuffer(const VulkanCore::VGraphicsPipeline& pipeline) = 0;

    protected:
        std::vector<std::unique_ptr<Renderer::RenderTarget>> m_renderTargets; // render to these images, per frame in flight
        std::vector<std::unique_ptr<VulkanCore::VCommandBuffer>> m_commandBuffers;
        const VulkanCore::VDevice& m_device;
        VulkanCore::VSyncPrimitive<vk::Semaphore> m_rendererFinishedSemaphore;

        int m_width, m_height;
    };
} // Renderer

#endif //BASERENDERER_HPP