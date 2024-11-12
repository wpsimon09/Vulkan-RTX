//
// Created by wpsimon09 on 12/11/24.
//

#include "VSampler.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"

namespace VulkanCore {

    VSampler::VSampler(const VulkanCore::VDevice &device):m_device(device) {
            Utils::Logger::LogInfo("Creating sampler...");
            vk::SamplerCreateInfo samplerInfo;
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
            samplerInfo.anisotropyEnable = true;
            auto maxAntistropy = GlobalVariables::GlobalStructs::GpuProperties.limits.maxSamplerAnisotropy ;
            assert(maxAntistropy > 0);
            samplerInfo.maxAnisotropy = maxAntistropy;
            samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
            samplerInfo.unnormalizedCoordinates = false;
            samplerInfo.compareEnable = false;
            samplerInfo.compareOp = vk::CompareOp::eAlways;
            samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.maxLod = 0.0f;
            samplerInfo.minLod = 0.0f;

            m_sampler = m_device.GetDevice().createSampler(samplerInfo);

            Utils::Logger::LogSuccess("Sampler created successfully!");
        }

    VSampler::VSampler(const VulkanCore::VDevice &device, vk::SamplerCreateInfo samplerCreateInfo):m_device(device) {
        m_sampler = m_device.GetDevice().createSampler(samplerCreateInfo);

        Utils::Logger::LogSuccess("Sampler created successfully!");
    }

    void VSampler::Destroy() {
        m_device.GetDevice().destroySampler(m_sampler);
    }

}// VulkanCore