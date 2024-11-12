//
// Created by wpsimon09 on 12/11/24.
//

#ifndef VSAMPLER_HPP
#define VSAMPLER_HPP


#include "vulkan/vulkan.hpp"

#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore {
class VDevice;

class VSampler: public VObject {
public:
    explicit VSampler(const VulkanCore::VDevice& device);
    explicit VSampler(const VulkanCore::VDevice& device, vk::SamplerCreateInfo samplerCreateInfo);

    const vk::Sampler& GetSamplerHandle() const {return m_sampler;};

    void Destroy() override;

private:
    vk::Sampler m_sampler;
    const VulkanCore::VDevice& m_device;
};

} // VulkanCore

#endif //VSAMPLER_HPP
