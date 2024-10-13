//
// Created by wpsimon09 on 13/10/24.
//

#ifndef COMMANDPOOL_HPP
#define COMMANDPOOL_HPP
#include <vulkan/vulkan.hpp>
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"

namespace VulkanCore {
class VDevice;

class VCommandPool:public VObject {
public:
    explicit VCommandPool(const VulkanCore::VDevice& device, QUEUE_FAMILY_INDEX_TYPE queueFamilyType);

    void Destroy() override;

    const vk::CommandPool& GetCommandBufferPool() const { return m_commandPool; };
    const std::pair<QUEUE_FAMILY_INDEX_TYPE, uint32_t>& GetQueueFamily() const{return m_queueFamilyIndex;}

    ~VCommandPool() = default;
private:
    void CreateCommandPool();

private:
    std::pair<QUEUE_FAMILY_INDEX_TYPE, uint32_t> m_queueFamilyIndex;
    const VulkanCore::VDevice& m_device;
    vk::CommandPool m_commandPool;
};

} // VulkanCore

#endif //COMMANDPOOL_HPP
