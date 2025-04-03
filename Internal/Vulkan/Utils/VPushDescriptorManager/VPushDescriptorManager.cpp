//
// Created by wpsimon09 on 08/11/24.
//

#include "VPushDescriptorManager.hpp"

#include "VPushDescriptor.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace VulkanUtils {
    VPushDescriptorManager::VPushDescriptorManager(const VulkanCore::VDevice &device): m_device(device) {

            VulkanUtils::UnlitSingleTexture unlitSingleTexture{};
            auto layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, unlitSingleTexture);
            m_pushDescriptors[EDescriptorLayoutStruct::UnlitSingleTexture] =
                std::make_shared<VPushDescriptorSet>(m_device, std::string("Unlit single texture"),std::move(layout));

            VulkanUtils::ForwardShadingDstSet forwardShading{};
            layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, forwardShading);
            m_pushDescriptors[EDescriptorLayoutStruct::ForwardShading] =
                std::make_shared<VPushDescriptorSet>(m_device, std::string("Forward shading dst set"), std::move(layout));

            VulkanUtils::BasicDescriptorSet basicDescriptorSet{};
            layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, basicDescriptorSet);
            m_pushDescriptors[EDescriptorLayoutStruct::Basic] =
                std::make_shared<VPushDescriptorSet>(m_device, std::string("Basic descriptor set"), std::move(layout));


    }


    void VPushDescriptorManager::Destroy() {
        for (auto& pushDst: m_pushDescriptors)
        {
            pushDst.second->Destroy();
        }
    }

} // VulkanUtils