//
// Created by wpsimon09 on 31/10/24.
//

#ifndef VUNIFORMBUFFERMANAGER_HPP
#define VUNIFORMBUFFERMANAGER_HPP
#include <memory>
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "UnifromsRegistry.hpp"
#include "VUniform.hpp"
class Client;

namespace VulkanUtils
{
    class VUniformBufferManager {
    public:
        VUniformBufferManager(const VulkanCore::VDevice& device, const Client& client);
        const std::vector<vk::DescriptorBufferInfo>& GetGlobalBufferDescriptorInfo() const;
        void UpdateAllUniformBuffers(int frameIndex) const;
        void Destroy() const;
    private:
        void CreateUniforms();
    private:
        const VulkanCore::VDevice& m_device;
        const Client& m_client;
        std::unique_ptr<VulkanUtils::VUniform<PerFrameUBO::CameraUniform>> m_cameraUniform;
    };
}




#endif //VUNIFORMBUFFERMANAGER_HPP
