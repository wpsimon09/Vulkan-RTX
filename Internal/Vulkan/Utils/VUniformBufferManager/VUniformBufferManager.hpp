//
// Created by wpsimon09 on 31/10/24.
//

#ifndef VUNIFORMBUFFERMANAGER_HPP
#define VUNIFORMBUFFERMANAGER_HPP
#include <memory>
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Material/Material.hpp"
#include "UnifromsRegistry.hpp"
#include "VUniform.hpp"

namespace VulkanStructs
{
    struct DrawCallData;
}

class Client;

namespace VulkanUtils
{
    class VUniformBufferManager {
    public:
        VUniformBufferManager(const VulkanCore::VDevice& device, const Client& client);
        const std::vector<vk::DescriptorBufferInfo>& GetGlobalBufferDescriptorInfo() const; // per frame in flight
        const std::vector<vk::DescriptorBufferInfo>& GetPerObjectDescriptorBufferInfo(int meshIndex) const; // per object per frame in flight
        void UpdateAllUniformBuffers(int frameIndex, std::vector<VulkanStructs::DrawCallData>& drawCalls) const;
        void Destroy() const;
    private:
        void CreateUniforms();
    private:
        const VulkanCore::VDevice& m_device;
        const Client& m_client;

        std::unique_ptr<VulkanUtils::VUniform<PerFrameUBO::GlobalUniform>> m_cameraUniform;
        std::vector<std::unique_ptr<VulkanUtils::VUniform<PerObjectUBO::ObjectDataUniform>>> m_objectDataUniforms;
        std::vector<std::unique_ptr<VulkanUtils::VUniform<PBRMaterialDescription>>> m_materialDataUniforms;
    };
}




#endif //VUNIFORMBUFFERMANAGER_HPP
