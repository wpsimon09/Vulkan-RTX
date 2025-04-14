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

class VCommandPool : public VObject
{
  public:
    explicit VCommandPool(const VulkanCore::VDevice& device, EQueueFamilyIndexType queueFamilyType);

    void Destroy() override;


    const vk::CommandPool&                            GetCommandBufferPool() const { return m_commandPool; };
    const std::pair<EQueueFamilyIndexType, uint32_t>& GetQueueFamily() const { return m_queueFamilyIndex; }
    bool                                              IsInUse() const { return m_inUse; }
    void                                              SetInUse(bool inUse) { m_inUse = inUse; }

    ~VCommandPool() = default;

  private:
    void CreateCommandPool();

  private:
    bool                                       m_inUse = false;
    std::pair<EQueueFamilyIndexType, uint32_t> m_queueFamilyIndex;
    const VulkanCore::VDevice&                 m_device;
    vk::CommandPool                            m_commandPool;
};

}  // namespace VulkanCore

#endif  //COMMANDPOOL_HPP
