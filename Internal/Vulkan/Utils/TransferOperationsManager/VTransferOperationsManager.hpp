//
// Created by wpsimon09 on 11/03/25.
//

#ifndef VTRANSFEROPERATIONSMANAGER_HPP
#define VTRANSFEROPERATIONSMANAGER_HPP
#include <memory>

namespace VulkanCore
{
    class VTimelineSemaphore;
    class VCommandBuffer;
}

namespace VulkanCore
{
    class VDevice;
}

namespace VulkanUtils {
    /**
     * Will operate only on transfer queues and record commands and submits for transfer operations to happen
     */
    class VTransferOperationsManager {

    public:
        VTransferOperationsManager(const VulkanCore::VDevice& device);

        VulkanCore::VCommandBuffer& GetCommandBuffer();
        void UpdateGPU();
    private:
        const VulkanCore::VDevice& m_device;
        std::unique_ptr<VulkanCore::VCommandBuffer> m_commandBuffer;
        std::unique_ptr<VulkanCore::VTimelineSemaphore> m_transferTimeline;
    };

} // VulkanUtils

#endif //VTRANSFEROPERATIONSMANAGER_HPP
