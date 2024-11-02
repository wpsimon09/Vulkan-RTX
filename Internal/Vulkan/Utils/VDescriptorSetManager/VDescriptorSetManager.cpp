//
// Created by wpsimon09 on 02/11/24.
//

#include "VDescriptorSetManager.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorPool.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorWriter.hpp"

namespace VulkanUtils {
    VDescriptorSetManager::VDescriptorSetManager(const VulkanCore::VDevice &device):m_device(device) {
        m_descriptorPoolGlobal = VulkanCore::VDescriptorPool::Builder(m_device)
        .AddMaxSets(GlobalVariables::MAX_FRAMES_IN_FLIGHT)
        .AddPoolsSize(vk::DescriptorType::eUniformBuffer, GlobalVariables::MAX_FRAMES_IN_FLIGHT)
        .Build();

        m_globalDescriptorLayout = VulkanCore::VDescriptorSetLayoutBuilder(m_device)
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Build();

    }

    void VDescriptorSetManager::CreateGlobalDescriptorSets(
        std::vector<vk::DescriptorBufferInfo> &bufferDescriptorInfo) {
        m_globalDescriptorSets.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
        assert(m_globalDescriptorSets.size() == bufferDescriptorInfo.size() && "Global uniform buffer descriptor size does not match the global uniform buffers size");
        for (size_t i = 0; i < m_globalDescriptorSets.size(); i++) {
            VulkanCore::VDescriptorWriter(*m_globalDescriptorLayout, *m_descriptorPoolGlobal)
                .WriteBuffer(0, &bufferDescriptorInfo[i])
                .Build(m_globalDescriptorSets[i]);
        }
    }

    void VDescriptorSetManager::Destroy() {
    }
} // VulkanUtils