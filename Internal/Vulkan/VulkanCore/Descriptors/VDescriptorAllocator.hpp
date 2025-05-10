//
// Created by wpsimon09 on 10/05/25.
//

#ifndef VDESCRIPTORALLOCATOR_HPP
#define VDESCRIPTORALLOCATOR_HPP

#include "vulkan/vulkan.hpp"

#include <vector>
// thin abstraction over descriptor sets based on  https://github.com/vblanco20-1/Vulkan-Descriptor-Allocator

namespace VulkanCore {

class VDescriptorAllocator
{
  public:
    struct PoolSizes
    {
        std::vector<std::pair<vk::DescriptorType, float>> sizes = {
          {vk::DescriptorType::eCombinedImageSampler, 0.5f},
          {vk::DescriptorType::eStorageImage, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},
          {vk::DescriptorType::eSampler, 0.5f},


        };
    };
};

class vDescriptorLayoutCache
{
};


}  // namespace VulkanCore

#endif  //VDESCRIPTORALLOCATOR_HPP
