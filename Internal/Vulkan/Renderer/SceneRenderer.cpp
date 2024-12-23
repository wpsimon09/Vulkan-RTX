//
// Created by wpsimon09 on 21/12/24.
//

#include "SceneRenderer.hpp"

#include "Vulkan/Utils/VPushDescriptorManager/VPushDescriptorManager.hpp"

namespace Renderer {

    SceneRenderer::SceneRenderer(const VulkanCore::VDevice& device,
        VulkanUtils::VPushDescriptorManager& pushDescriptorManager): m_device(device), m_pushDescriptorManager(pushDescriptorManager), BaseRenderer(device)
    {
        // create render target

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
} // Renderer