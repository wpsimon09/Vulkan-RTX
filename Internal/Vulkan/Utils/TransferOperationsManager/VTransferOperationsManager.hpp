//
// Created by wpsimon09 on 11/03/25.
//

#ifndef VTRANSFEROPERATIONSMANAGER_HPP
#define VTRANSFEROPERATIONSMANAGER_HPP
#include <memory>
#include <vector>
#include "VMA/vk_mem_alloc.h"
#include "Application/Structs/ApplicationStructs.hpp"

namespace VulkanCore {
class VBuffer;
class VTimelineSemaphore;
class VCommandBuffer;
}  // namespace VulkanCore

namespace VulkanCore {
class VDevice;
}

namespace VulkanUtils {
/**
     * Will operate only on transfer queues and record commands and submits for transfer operations to happen
     */
class VTransferOperationsManager
{

  public:
    VTransferOperationsManager(const VulkanCore::VDevice& device);

    VulkanCore::VCommandBuffer& GetCommandBuffer();
    void                        StartRecording();
    void                        UpdateGPU();
    void                        UpdateGPUWaitCPU(bool startRecording = false);

    void ClearResources();

    void DestroyBuffer(VkBuffer& buffer, VmaAllocation& vmaAllocation);
    void DestroyBuffer(VulkanCore::VBuffer& vBuffer, bool isStaging = false);

    VulkanCore::VTimelineSemaphore& GetTransferSemaphore() const { return *m_transferTimeline; }

    void Destroy();

  private:
    bool                                            m_hasPandingWork = false;
    const VulkanCore::VDevice&                      m_device;
    std::unique_ptr<VulkanCore::VCommandBuffer>     m_commandBuffer;
    std::unique_ptr<VulkanCore::VTimelineSemaphore> m_transferTimeline;

    std::vector<std::pair<VkBuffer, VmaAllocation>>    m_clearBuffersVKVMA;
    std::vector<std::pair<bool, VulkanCore::VBuffer*>> m_clearVBuffers;
};

}  // namespace VulkanUtils

#endif  //VTRANSFEROPERATIONSMANAGER_HPP
