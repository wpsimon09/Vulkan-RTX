//
// Created by wpsimon09 on 21/12/24.
//

#ifndef USERINTERFACERENDERER_HPP
#define USERINTERFACERENDERER_HPP
#include "BaseRenderer.hpp"


namespace VulkanUtils
{
    class ImGuiInitializer;
}

namespace Renderer
{
    class UserInterfaceRenderer{
    public:
        explicit UserInterfaceRenderer(
            const VulkanCore::VDevice& device,
            VulkanCore::VSwapChain* swapChain,
            VulkanUtils::ImGuiInitializer& imGuiInitilaizer);

        void RenderAndPresnet(int currentFrameIndex);

    private:
        const VulkanCore::VDevice& m_device;
        std::unique_ptr<Renderer::RenderTarget> m_renderTarget;
        VulkanUtils::ImGuiInitializer& m_imguiInitializer;
    private:

        void CreateRenderTargets(VulkanCore::VSwapChain* swapChain);

        void RecordCommandBuffer(
            int currentFrameIndex,
            const VulkanUtils::VUniformBufferManager& uniformBufferManager,
            const VulkanCore::VGraphicsPipeline& pipeline);

        std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>> m_Semaphore;

    };
}



#endif //USERINTERFACERENDERER_HPP
