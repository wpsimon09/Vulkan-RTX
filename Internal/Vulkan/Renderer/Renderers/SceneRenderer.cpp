//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"

namespace Renderer {


    SceneRenderer::SceneRenderer(const VulkanCore::VDevice& device,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager, int width, int height): BaseRenderer(device),
        m_pushDescriptorManager(pushDescriptorManager),
        m_device(m_device)
    {

        m_width = width;
        m_height = height;
        SceneRenderer::CreateRenderTargets();

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

    void SceneRenderer::CreateRenderTargets(std::optional<VulkanCore::VSwapChain&>)
    {
        m_renderTargets.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_renderTargets[i] = std::make_unique<Renderer::RenderTarget>(m_device,m_width, m_height);
        }
    }
} // Renderer