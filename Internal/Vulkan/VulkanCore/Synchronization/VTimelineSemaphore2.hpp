//
// Created by wpsimon09 on 02/09/2025.
//

#ifndef VULKAN_RTX_VTIMELINESEMAPHORE2_HPP
#define VULKAN_RTX_VTIMELINESEMAPHORE2_HPP
#include "VTimelineSemaphore.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore {
class VDevice;

class VTimelineSemaphore2 : public VObject
{
public:
  VTimelineSemaphore2(const VulkanCore::VDevice& device, uint32_t maxStages);

  vk::SemaphoreSubmitInfo         GetSemaphoreWaitSubmitInfo(uint32_t stage, vk::PipelineStageFlags2 waitStages);
  vk::SemaphoreSubmitInfo         GetSemaphoreSignalSubmitInfo(uint32_t stage, vk::PipelineStageFlags2 signalStages);
  void ProcedeToNextFrame();

  void CpuSignal  (uint32_t signalStage);
  void CpuWaitIdle(uint32_t waitStage);

  void Reset();

  void Destroy() override;

  uint64_t Frame = 0;

private:
  uint32_t GetStageValue(uint32_t stage) const;

  const VulkanCore::VDevice& m_device;
  vk::Semaphore m_semaphore;
  uint64_t m_frame = 0;
  uint32_t m_maxStageValue = 1;
};

}  // namespace VulkanCore

#endif  //VULKAN_RTX_VTIMELINESEMAPHORE2_HPP
