//
// Created by wpsimon09 on 12/11/24.
//

#ifndef VSAMPLERS_HPP
#define VSAMPLERS_HPP

#include "vulkan/vulkan.hpp"

namespace VulkanCore
{
    class VDevice;
}
namespace VulkanCore::VSamplers
{
    inline vk::Sampler Sampler2D;

    inline vk::Sampler SamplerClampToEdge;

    inline vk::Sampler SamplerCube;

    void CreateAllSamplers(const VulkanCore::VDevice& device);

    void DestroyAllSamplers(const VulkanCore::VDevice& device);

    void GetSamplerDescriptorInfo(const vk::Sampler &sampler);
}

#endif //VSAMPLERS_HPP
