//
// Created by wpsimon09 on 23/12/24.
//

#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"

namespace VulkanUtils
{
    class ImGuiInitializer;
}

namespace VulkanUtils
{
    class VUniformBufferManager;
}

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

        const VulkanCore::VRenderPass& GetRenderPass(int currentFrame ) const {return *m_renderTargets->m_renderPass;};
        const VulkanCore::VImage& GetRenderedImage(int currentFrame ) const {return *m_renderTargets->m_colourBuffer[currentFrame];}; // i have to place fence to access the image
        const VulkanCore::VFrameBuffer& GetFrameBuffer(int currentFrame ) const {return *m_renderTargets->m_frameBuffers[currentFrame];};
        const int& GetTargeWidth() const  {return m_width;}
        const int& GetTargeHeight() const {return m_height;}
        virtual void Destroy();
        virtual void Render(int currentFrameIndex,
                            const VulkanCore::VSyncPrimitive<vk::Fence>& renderingFinishedFence,
                            GlobalUniform& globalUniformUpdateInfo,
                            const VulkanUtils::VUniformBufferManager& uniformBufferManager ,
                            const VulkanStructs::RenderContext& renderContext,
                            const VulkanCore::VGraphicsPipeline& pipeline) {};
        virtual void Render(int currentFrameIndex, const VulkanUtils::ImGuiInitializer& guiInitialzer) {}
    protected:
        virtual void CreateRenderTargets(VulkanCore::VSwapChain* swapChain = nullptr) = 0;
        virtual void RecordCommandBuffer(int currentFrameIndex,
                                        const VulkanUtils::VUniformBufferManager& uniformBufferManager ,
                                        const VulkanCore::VGraphicsPipeline& pipeline) = 0;

    protected:
        std::unique_ptr<Renderer::RenderTarget> m_renderTargets; // render to these images, per frame in flight
        std::vector<std::unique_ptr<VulkanCore::VCommandBuffer>> m_commandBuffers;
        const VulkanCore::VDevice& m_device;
        std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_rendererFinishedSemaphore;

        int m_width, m_height;
    };
} // Renderer

#endif //BASERENDERER_HPP
