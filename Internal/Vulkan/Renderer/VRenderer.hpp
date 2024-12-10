//
// Created by wpsimon09 on 15/10/24.
//

#ifndef VRENDERER_HPP
#define VRENDERER_HPP
#include <memory>
#include <glm/glm.hpp>

#include "Vulkan/VulkanCore/Synchronization/VSyncPrimitive.hpp"
#include <vulkan/vulkan.hpp>

#include "Vulkan/Global/VulkanStructs.hpp"

namespace VulkanUtils
{
    class VPushDescriptorManager;
}

namespace VulkanUtils
{
    class VDescriptorSetManager;
    class VUniformBufferManager;
}

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

namespace Renderer
{

    class VRenderer
    {
    public:
        VRenderer(const VulkanCore::VulkanInstance &instance, const VulkanCore::VDevice &device, Client &client,
                  const VulkanUtils::VUniformBufferManager &uniformBufferManager, VulkanUtils::VPushDescriptorManager& pushDescriptorSetManager);

        void Render(GlobalUniform& globalUniformUpdateInfo) ;
        void Destroy();
        ~VRenderer() = default;
    public:
        void SetRtxStatus(bool newStatus) {m_isRTXOn = newStatus;}
        VulkanStructs::RenderContext* GetRenderingContext() const{return m_renderingContext;}

    private:
        //==================================
        // FOR INITIALIZATION
        //==================================
        void CreateCommandBufferPools();
        void CreateSyncPrimitives();
        void CreateTemplateEntries();
        //==================================

        //==================================
        // FOR COMMAND BUFFER
        //==================================
        void StartRenderPass();
        void RecordCommandBuffersForPipelines(const vk::Pipeline &pipeline);
        void PushDescriptors();
        void EndRenderPass();
        //====================================

        //=====================================
        // FOR MAIN LOOP
        //=====================================
        vk::Result FetchSwapChainImage();
        void SubmitCommandBuffer();
        void PresentResults();
        //=====================================
    private:
        const VulkanCore::VDevice &m_device;
        const VulkanUtils::VUniformBufferManager &m_uniformBufferManager;
        VulkanUtils::VPushDescriptorManager &m_pushDescriptorSetManager;
        uint32_t m_currentImageIndex = 0;
        uint32_t m_currentFrameIndex = 0;
        int m_availableRecordingThreads = 0;
        bool m_isRTXOn = false;

        std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_imageAvailableSemaphores;
        std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Semaphore>>> m_renderFinishedSemaphores;
        std::vector<std::unique_ptr<VulkanCore::VSyncPrimitive<vk::Fence>>> m_isFrameFinishFences;

        std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;
        std::unique_ptr<class VulkanCore::VPipelineManager> m_pipelineManager;
        std::unique_ptr<class VulkanCore::VRenderPass> m_mainRenderPass;
        std::unique_ptr<class VulkanCore::VCommandPool> m_baseCommandPool;
        std::vector<std::unique_ptr<class VulkanCore::VCommandBuffer>> m_baseCommandBuffers;
        std::unique_ptr<class VulkanCore::VImage> m_depthBuffer;
        const VulkanCore::VGraphicsPipeline *m_graphicsPipeline;

        VulkanStructs::RenderContext m_rasterRenderContext;
        VulkanStructs::RenderContext m_rayTracingRenderContext;

        // points to one of the rendering contexts
        VulkanStructs::RenderContext* m_renderingContext;;


    };
} // Renderer

#endif //VRENDERER_HPP
