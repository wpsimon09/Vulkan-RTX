//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"


namespace Renderer {


    SceneRenderer::SceneRenderer(const VulkanCore::VDevice& device,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager, int width, int height): BaseRenderer(device),
        m_pushDescriptorManager(pushDescriptorManager),
        m_device(device)
    {

        m_width = width;
        m_height = height;
        SceneRenderer::CreateRenderTargets(nullptr);

        //---------------------------------------------------------------------------------------------------------------------------
        // CREATING TEMPLATE ENTRIES
        //---------------------------------------------------------------------------------------------------------------------------
        // global
        m_pushDescriptorManager.AddUpdateEntry(0, offsetof(VulkanUtils::DescriptorSetData, cameraUBOBuffer), 0);
        // per object
        m_pushDescriptorManager.AddUpdateEntry(1, offsetof(VulkanUtils::DescriptorSetData, meshUBBOBuffer), 0);
        // per material
        m_pushDescriptorManager.AddUpdateEntry(2, offsetof(VulkanUtils::DescriptorSetData, pbrMaterialNoTexture), 0);
        m_pushDescriptorManager.AddUpdateEntry(3, offsetof(VulkanUtils::DescriptorSetData, pbrMaterialFeatures), 0);
        m_pushDescriptorManager.AddUpdateEntry(4, offsetof(VulkanUtils::DescriptorSetData, diffuseTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(5, offsetof(VulkanUtils::DescriptorSetData, normalTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(6, offsetof(VulkanUtils::DescriptorSetData, armTextureImage), 0);
        m_pushDescriptorManager.AddUpdateEntry(7, offsetof(VulkanUtils::DescriptorSetData, emissiveTextureImage), 0);
    }

    void SceneRenderer::CreateRenderTargets(VulkanCore::VSwapChain* swapChain)
    {
        m_renderTargets.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_renderTargets[i] = std::make_unique<Renderer::RenderTarget>(m_device,m_width, m_height);
        }
    }


    void SceneRenderer::Render(int currentFrameIndex, GlobalUniform& globalUniformUpdateInfo,
        const VulkanStructs::RenderContext& renderContext, const VulkanCore::VGraphicsPipeline& pipeline)
    {
        auto &renderTarget = m_renderTargets[currentFrameIndex];
        m_commandBuffers[currentFrameIndex]->Reset();

        //=====================================================
        // STARTING THE RENDER PASS
        //=====================================================
        RecordCommandBuffer(currentFrameIndex, pipeline);

        //===============================================================


    }

    void SceneRenderer::RecordCommandBuffer(int currentFrameIndex, const VulkanCore::VGraphicsPipeline& pipeline)
    {

        m_commandBuffers[currentFrameIndex]->BeginRecording();

        //==============================================
        // CREATAE RENDER PASS INFO
        //==============================================
        vk::RenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.renderPass = GetRenderPass(currentFrameIndex).GetRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(GetTargeWidth()),
        renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(GetTargeHeight());

        //==============================================
        // CONFIGURE CLEAR
        //==============================================
        std::array<vk::ClearValue,2> clearColors = {};
        clearColors[0].color =  {0.2f, 0.2f, 0.2f, 1.0f};
        clearColors[1].depthStencil.depth = 1.0f;
        clearColors[1].depthStencil.stencil = 0.0f;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
        renderPassBeginInfo.pClearValues = clearColors.data();

        //==============================================
        // START RENDER PASS
        //==============================================
        m_commandBuffers[currentFrameIndex]->GetCommandBuffer().beginRenderPass(
            &renderPassBeginInfo, vk::SubpassContents::eInline);
    }
} // Renderer