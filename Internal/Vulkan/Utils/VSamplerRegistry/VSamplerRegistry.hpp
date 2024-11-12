//
// Created by wpsimon09 on 12/11/24.
//

#ifndef VSAMPLERREGISTERY_HPP
#define VSAMPLERREGISTERY_HPP
#include <map>
#include <memory>
#include "vulkan/vulkan.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"

namespace VulkanCore
{
    class VDevice;
    class VSampler;
}

namespace VulkanUtils {

class VSamplerRegistry {
public:
    explicit VSamplerRegistry(const VulkanCore::VDevice& device);

    const VulkanCore::VSampler& GetSamplerOfType(SAMPLER_TYPE type) const;

    const vk::Sampler &GetSamplerHandleOfType(SAMPLER_TYPE type) const;

private:
    const VulkanCore::VDevice& m_device;
    std::map<SAMPLER_TYPE, std::unique_ptr<VulkanCore::VSampler>> m_samplers;

    void CreateBaseSampler();
};

} // VulkanUtils

#endif //VSAMPLERREGISTERY_HPP
