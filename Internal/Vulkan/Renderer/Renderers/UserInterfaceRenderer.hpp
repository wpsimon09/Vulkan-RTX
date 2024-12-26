//
// Created by wpsimon09 on 21/12/24.
//

#ifndef USERINTERFACERENDERER_HPP
#define USERINTERFACERENDERER_HPP
#include <memory>
#include <vector>

#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"


namespace VulkanCore
{
    class VCommandBuffer;
}

namespace VulkanCore
{
    class VGraphicsPipeline;
    class VCommandPool;
    class VSwapChain;
    class VDevice;
}

namespace VulkanUtils
{
    class VUniformBufferManager;
    class ImGuiInitializer;
}

namespace Renderer
{
    class RenderTarget;

    class UserInterfaceRenderer{
    public:
        explicit UserInterfaceRenderer(
            const VulkanCore::VDevice& device,
            const VulkanCore::VSwapChain& swapChain,
            VulkanUtils::ImGuiInitializer& imGuiInitilaizer);

            void RenderAndPresent(int currentFrameIndex,int swapChainImageIndex, const VulkanCore::VSyncPrimitive<vk::Fence>& renderingFinishedFence,  std::vector<std::pair<vk::Semaphore, vk::PipelineStageFlags>>& waitSemaphores);

            RenderTarget& GetRenderTarget() const {return *m_renderTarget;};
    private:
        const VulkanCore::VDevice& m_device;

        std::unique_ptr<Renderer::RenderTarget> m_renderTarget;
        std::unique_ptr<VulkanCore::VCommandPool> m_commandPool;
        std::vector<std::unique_ptr<VulkanCore::VCommandBuffer>> m_commandBuffer;

        VulkanUtils::ImGuiInitializer& m_imguiInitializer;
    private:
        void RecordCommandBuffer(int currentFrameIndex);

        std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_ableToPresentSemaphore;
    };
}



#endif //USERINTERFACERENDERER_HPP
