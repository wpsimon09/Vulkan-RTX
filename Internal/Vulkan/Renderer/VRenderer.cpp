//
// Created by wpsimon09 on 15/10/24.
//

#include "VRenderer.hpp"

#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Application/Client.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Utils/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"


namespace Renderer {

    VRenderer::VRenderer(const VulkanCore::VulkanInstance &instance, const VulkanCore::VDevice &device,
        const Client &client):m_device(device), m_client(client) {
        m_swapChain = std::make_unique<VulkanCore::VSwapChain>(device, instance);
        m_mainRenderPass = std::make_unique<VulkanCore::VRenderPass>(device, *m_swapChain);
        m_pipelineManager = std::make_unique<VulkanCore::VPipelineManager>(device, *m_swapChain, *m_mainRenderPass);
        m_pipelineManager->InstantiatePipelines();
        m_swapChain->CreateSwapChainFrameBuffers(*m_mainRenderPass);
        m_renderingCommandPool = std::make_unique<VulkanCore::VCommandPool>(device, QUEUE_FAMILY_INDEX_GRAPHICS);
        m_renderingCommandBuffer = std::make_unique<VulkanCore::VCommandBuffer>(device, *m_renderingCommandPool);
    }

    void VRenderer::Destroy() {
        m_mainRenderPass->Destroy();
        m_pipelineManager->DestroyPipelines();
        m_renderingCommandBuffer->Destroy();
        m_renderingCommandPool->Destroy();
        m_swapChain->Destroy();
    }
} // Renderer