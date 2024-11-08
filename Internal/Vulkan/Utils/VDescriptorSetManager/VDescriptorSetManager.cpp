//
// Created by wpsimon09 on 02/11/24.
//

#include "VDescriptorSetManager.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorPool.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSet.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSet.hpp"

namespace VulkanUtils {
    VDescriptorSetManager::VDescriptorSetManager(const VulkanCore::VDevice &device):m_device(device) {
        m_globalDescriptorPool = VulkanCore::VDescriptorPool::Builder(m_device)
        .AddMaxSets(GlobalVariables::MAX_FRAMES_IN_FLIGHT)
        .AddPoolsSize(vk::DescriptorType::eUniformBuffer, GlobalVariables::MAX_FRAMES_IN_FLIGHT)
        .Build();



        m_globalDescriptorLayout = VulkanCore::VDescriptorSetLayout::Builder(m_device)
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Build();
    }

    void VDescriptorSetManager::CreateGlobalDescriptorSets  (
        const std::vector<vk::DescriptorBufferInfo> &bufferDescriptorInfo){
        m_globalDescriptorSet = std::make_unique<VulkanCore::VDescriptorSet>(*m_globalDescriptorLayout, *m_globalDescriptorPool);
        m_globalDescriptorSet->WriteBuffer(0, bufferDescriptorInfo);
    }

    const VulkanCore::VDescriptorSetLayout& VDescriptorSetManager::GetGlobalDescriptorSetLayout() const {
        return *m_globalDescriptorLayout;
    }

    void VDescriptorSetManager::Destroy() {

    }
} // VulkanUtils