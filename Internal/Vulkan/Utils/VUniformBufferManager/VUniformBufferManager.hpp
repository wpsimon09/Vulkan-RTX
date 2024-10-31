//
// Created by wpsimon09 on 31/10/24.
//

#ifndef VUNIFORMBUFFERMANAGER_HPP
#define VUNIFORMBUFFERMANAGER_HPP
#include <memory>
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "UnifromsRegistry.hpp"

class Client;

namespace VulkanUtils
{
    class VUniformBufferManager {
        VUniformBufferManager(const VulkanCore::VDevice& device, const Client& client);

    private:
        void CreateUniforms();
    private:
        const VulkanCore::VDevice& m_device;
        const Client& m_client;
        std::unique_ptr<PerFrameUBO::CameraUniform> m_cameraUniform;
    };
}




#endif //VUNIFORMBUFFERMANAGER_HPP
