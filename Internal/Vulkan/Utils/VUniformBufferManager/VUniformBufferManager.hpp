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
        VUniformBufferManager(const VulkanCore::VDevice& device);

        const std::vector<vk::DescriptorBufferInfo>& GetGlobalBufferDescriptorInfo() const; // per frame in flight
        const std::vector<vk::DescriptorBufferInfo>& GetLightBufferDescriptorInfo() const;
        const std::vector<vk::DescriptorBufferInfo>& GetMaterialFeaturesDescriptorBufferInfo(int meshIndex) const; // per object per frame in flight
        const std::vector<vk::DescriptorBufferInfo>& GetPerMaterialNoMaterialDescrptorBufferInfo(int meshIndex) const; // per object per frame in flight
        const std::vector<vk::DescriptorBufferInfo>& GetPerObjectDescriptorBufferInfo(int meshIndex) const; // per object per frame in flight

        void UpdatePerFrameUniformData(int frameIndex, GlobalUniform& perFrameData) const;

        void UpdatePerObjectUniformData(int frameIndex, std::vector<std::pair<unsigned short, VulkanStructs::DrawCallData>>& drawCalls) const;

        void UpdateLightUniformData(int frameIndex, LightStructs::SceneLightInfo& sceneLightInfo) const;

        void UpdatePerMaterialUniformData(int frameIndex, const std::shared_ptr<ApplicationCore::Material>& material) const;

        void Destroy() const;
    private:
        void CreateUniforms();
    private:
        const VulkanCore::VDevice& m_device;

        std::unique_ptr<VulkanUtils::VUniform<LightUniforms>> m_lightUniform;
        std::unique_ptr<VulkanUtils::VUniform<GlobalUniform>> m_perFrameUniform;
        std::vector<std::unique_ptr<VulkanUtils::VUniform<ObjectDataUniform>>> m_perObjectUniform;
        std::vector<std::unique_ptr<VulkanUtils::VUniform<PBRMaterialFeaturees>>> m_materialFeaturesUniform;
        std::vector<std::unique_ptr<VulkanUtils::VUniform<PBRMaterialNoTexture>>> m_materialNoTextureUniform;

    };
}




#endif //VUNIFORMBUFFERMANAGER_HPP
