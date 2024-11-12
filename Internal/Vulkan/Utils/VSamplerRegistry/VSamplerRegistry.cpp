//
// Created by wpsimon09 on 12/11/24.
//

#include "VSamplerRegistry.hpp"

#include <cassert>

#include "Vulkan/VulkanCore/VImage/VSampler.hpp"

namespace VulkanUtils {
    VSamplerRegistry::VSamplerRegistry(const VulkanCore::VDevice &device):m_device(device) {

    }

    const VulkanCore::VSampler & VSamplerRegistry::GetSamplerOfType(SAMPLER_TYPE type) const {
        auto foundSampler = m_samplers.find(type);
        assert(foundSampler != m_samplers.end());
        return *foundSampler->second;
    }

    const vk::Sampler & VSamplerRegistry::GetSamplerHandleOfType(SAMPLER_TYPE type) const{
        return GetSamplerOfType(type).GetSamplerHandle();
    }

    void VSamplerRegistry::CreateBaseSampler() {
    }
} // VulkanUtils