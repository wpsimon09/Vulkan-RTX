//
// Created by wpsimon09 on 12/11/24.
//

#include "VSamplers.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"

namespace VulkanCore::VSamplers {

void CreateAllSamplers(const VulkanCore::VDevice& device)
{

  //---------------------------------------------------------
  //  SAMPLER 2D
  //---------------------------------------------------------
  Utils::Logger::LogInfoVerboseOnly("Creating sampler...");
  vk::SamplerCreateInfo samplerInfo;
  samplerInfo.magFilter        = vk::Filter::eLinear;
  samplerInfo.minFilter        = vk::Filter::eLinear;
  samplerInfo.addressModeU     = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeV     = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeW     = vk::SamplerAddressMode::eRepeat;
  samplerInfo.anisotropyEnable = true;
  auto maxAntistropy           = GlobalVariables::GlobalStructs::GpuProperties.properties.limits.maxSamplerAnisotropy;
  assert(maxAntistropy > 0
         || GlobalVariables::GlobalStructs::GpuProperties.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu);
  samplerInfo.maxAnisotropy           = maxAntistropy;
  samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
  samplerInfo.unnormalizedCoordinates = false;
  samplerInfo.compareEnable           = false;
  samplerInfo.compareOp               = vk::CompareOp::eAlways;
  samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
  samplerInfo.mipLodBias              = 0.0f;
  samplerInfo.maxLod                  = 0.0f;
  samplerInfo.minLod                  = 0.0f;
  VulkanCore::VSamplers::Sampler2D    = device.GetDevice().createSampler(samplerInfo);

  //---------------------------------------------------------
  //  SAMPLER 2D - CLAMP TO EDGE
  //---------------------------------------------------------
  Utils::Logger::LogInfoVerboseOnly("Creating sampler...");
  samplerInfo.magFilter                     = vk::Filter::eLinear;
  samplerInfo.minFilter                     = vk::Filter::eLinear;
  samplerInfo.addressModeU                  = vk::SamplerAddressMode::eClampToEdge;
  samplerInfo.addressModeV                  = vk::SamplerAddressMode::eClampToEdge;
  samplerInfo.addressModeW                  = vk::SamplerAddressMode::eClampToEdge;
  samplerInfo.anisotropyEnable              = false;
  samplerInfo.borderColor                   = vk::BorderColor::eIntOpaqueBlack;
  samplerInfo.unnormalizedCoordinates       = false;
  samplerInfo.compareEnable                 = false;
  samplerInfo.compareOp                     = vk::CompareOp::eAlways;
  samplerInfo.mipmapMode                    = vk::SamplerMipmapMode::eLinear;
  samplerInfo.mipLodBias                    = 0.0f;
  samplerInfo.minLod                        = 0.0f;
  samplerInfo.maxLod                        = 1.0f;
  VulkanCore::VSamplers::SamplerClampToEdge = device.GetDevice().createSampler(samplerInfo);

  samplerInfo.minLod                  = 0.0f;
  samplerInfo.maxLod                  = 5;
  VulkanCore::VSamplers::Sampler5Mips = device.GetDevice().createSampler(samplerInfo);

  samplerInfo.minLod                   = 0.0f;
  samplerInfo.maxLod                   = 10;
  VulkanCore::VSamplers::Sampler10Mips = device.GetDevice().createSampler(samplerInfo);

  samplerInfo.minLod                   = 0.0f;
  samplerInfo.maxLod                   = 20;
  VulkanCore::VSamplers::Sampler20Mips = device.GetDevice().createSampler(samplerInfo);


  Utils::Logger::LogSuccess("Sampler created successfully !");
}

void DestroyAllSamplers(const VulkanCore::VDevice& device)
{
  Utils::Logger::LogInfoVerboseOnly("Destroying samplers...");
  device.GetDevice().destroySampler(VulkanCore::VSamplers::Sampler2D);
  device.GetDevice().destroySampler(VulkanCore::VSamplers::Sampler5Mips);
  device.GetDevice().destroySampler(VulkanCore::VSamplers::Sampler10Mips);
  device.GetDevice().destroySampler(VulkanCore::VSamplers::Sampler20Mips);
  device.GetDevice().destroySampler(VulkanCore::VSamplers::SamplerCube);
  Utils::Logger::LogSuccess("Sampler destroyed successfully");
}

void GetSamplerDescriptorInfo(const vk::Sampler& sampler) {}
}  // namespace VulkanCore::VSamplers
