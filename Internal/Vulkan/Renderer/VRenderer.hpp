//
// Created by wpsimon09 on 15/10/24.
//

#ifndef VRENDERER_HPP
#define VRENDERER_HPP
#include <memory>

#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"
#include <vulkan/vulkan.hpp>

namespace VulkanCore
{
    class VBuffer;
}

class Client;
namespace VulkanCore
{
    class VGraphicsPipeline;
    class VulkanInstance;
    class VDevice;
    class VCommandBuffer;
    class VCommandPool;
    class VRenderPass;
    class VPipelineManager;
    class VSwapChain;

}

namespace Renderer {

class VRenderer {
public:
    VRenderer(const VulkanCore::VulkanInstance &instance, const VulkanCore::VDevice& device, const Client& client);
    void Render();
    void Destroy();
    ~VRenderer() = default;
private:
    //==================================
    // FOR INITIALIZATION
    //==================================
    void CreateCommandBufferPools();
    //==================================

    //==================================
    // FOR COMMAND BUFFER
    //==================================
    void StartRenderPass();
    void RecordCommandBuffersForPipelines();
    void EndRenderPass();
    void CreateSyncPrimitives();
    //====================================

    //=====================================
    // FOR MAIN LOOP
    //=====================================
    void FetchSwapChainImage();
    void SubmitCommandBuffer();
    void PresentResults();
    //=====================================

private:
    const Client& m_client;
    const VulkanCore::VDevice& m_device;
    uint32_t m_currentImageIndex = 0;
    int m_availableRecordingThreads = 0;

    std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>> m_imageAvailableSemaphore;
    std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>> m_renderFinishedSemaphore;
    std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Fence>> m_isFrameFinishFence;

    std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;
    std::unique_ptr<class VulkanCore::VPipelineManager> m_pipelineManager;
    std::unique_ptr<class VulkanCore::VRenderPass> m_mainRenderPass;
    std::unique_ptr<class VulkanCore::VCommandPool> m_baseCommandPool;
    std::unique_ptr<class VulkanCore::VCommandBuffer> m_baseCommandBuffer;
    const VulkanCore::VGraphicsPipeline* m_graphicsPipeline;

};

} // Renderer

#endif //VRENDERER_HPP
