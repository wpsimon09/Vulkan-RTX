//
// Created by wpsimon09 on 12/11/24.
//

#include "VSamplers.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"

namespace VulkanCore::VSamplers
{

    void CreateAllSamplers(const VulkanCore::VDevice &device) {

        //---------------------------------------------------------
        //  SAMPLER 2D
        //---------------------------------------------------------
        Utils::Logger::LogInfoVerboseOnly("Creating sampler...");
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

        VulkanCore::VSamplers::Sampler2D =  device.GetDevice().createSampler(samplerInfo);

        Utils::Logger::LogSuccess("Sampler created successfully !");
    }

    void DestroyAllSamplers(const VulkanCore::VDevice &device) {
        Utils::Logger::LogInfoVerboseOnly("Destroying samplers...");
        device.GetDevice().destroySampler(VulkanCore::VSamplers::Sampler2D);
        Utils::Logger::LogSuccess("Sampler destroyed successfully");
    }

    void GetSamplerDescriptorInfo(const vk::Sampler& sampler)
    {

    }
}