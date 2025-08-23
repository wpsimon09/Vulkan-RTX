//
// Created by wpsimon09 on 23/08/2025.
//

#ifndef VULKAN_RTX_VGROWABLEBUFFER_HPP
#define VULKAN_RTX_VGROWABLEBUFFER_HPP

#include "Vulkan/VulkanCore/VObject.hpp"


#include <vulkan/vulkan.hpp>
namespace VulkanCore {
static constexpr int SIZE_4_MB = 4194304;
static constexpr int SIZE_8_MB = 8388608;
static constexpr int SIZE_16_MB = 16777216;
static constexpr int SIZE_32_MB = 33554432;
static constexpr int SIZE_64_MB = 16777264;

class VDevice;

class VGrowableBuffer : public VulkanCore::VObject
{
public:
  VGrowableBuffer(const VulkanCore::VDevice& device, vk::DeviceSize initialSize, vk::DeviceSize chunkSize = SIZE_8_MB);
  void Allocate(vk::BufferUsageFlags* usage);
  template <typename T>
  void Fill(const T& data);

  template <typename T>
  void PushBack(const T& data);

  void Remove(vk::DeviceSize offset, vk::DeviceSize size);

  void Destroy() override;


private:


};

}  // namespace VulkanCore

#endif  //VULKAN_RTX_VGROWABLEBUFFER_HPP
