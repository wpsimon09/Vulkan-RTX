//
// Created by wpsimon09 on 29/10/24.
//

#ifndef VDESCRIPTORPOOL_HPP
#define VDESCRIPTORPOOL_HPP
#include <memory>

#include "Vulkan/VulkanCore/VObject.hpp"
#include "vulkan/vulkan.hpp"

namespace VulkanCore {
    class VDescriptorPool;
    class VDevice;
    class VDescriptorWriter;

class VDescriptorPool: public VObject {
    class Builder
    {
        public:
            explicit Builder(const VulkanCore::VDevice& device);
            Builder& AddPoolsSize(vk::DescriptorType type, uint32_t count);
            Builder& AddPoolFlag(vk::DescriptorPoolCreateFlags flags);
            Builder& AddMaxSets(uint32_t count);

            std::unique_ptr<VDescriptorPool> Build() const;
        private:
            std::vector<vk::DescriptorPoolSize> m_poolSizes{};
            uint32_t m_maxSets = 1000;
            vk::DescriptorPoolCreateFlags m_poolFlags;
            const VulkanCore::VDevice& m_device;
    };

public:
    explicit VDescriptorPool(const VulkanCore::VDevice& device, uint32_t maxSets, vk::DescriptorPoolCreateFlags flags, const std::vector<vk::DescriptorPoolSize> &poolSizes);
    void AllocateDescriptor(vk::DescriptorSetLayout layout, vk::DescriptorSet& descriptorSet);
    void FreeDescriptor(std::vector<vk::DescriptorSet> &descriptorSets);
    void ResetPool();
    void Destroy() override;
private:
    const VulkanCore::VDevice& m_device;
    vk::DescriptorPool m_descriptorPool;

    friend class VulkanCore::VDescriptorWriter;

};


} // VulkanCore

#endif //VDESCRIPTORPOOL_HPP
