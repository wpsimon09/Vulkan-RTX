//
// Created by wpsimon09 on 31/10/24.
//

#ifndef VUNIFORMBUFFERMANAGER_HPP
#define VUNIFORMBUFFERMANAGER_HPP
#include <memory>

#include "VUniform.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"


namespace VulkanCore {
class VShaderStorageBuffer;
}
namespace ApplicationCore {
struct SceneData;
}
struct PBRMaterialNoTexture;
struct PBRMaterialFeaturees;

namespace VulkanStructs {
struct VDrawCallData;
}

struct GlobalUniform;
struct LightUniforms;

namespace LightStructs {
struct SceneLightInfo;
}

class Client;

namespace VulkanUtils {
class VUniformBufferManager
{
  public:
    VUniformBufferManager(const VulkanCore::VDevice& device);

    const std::vector<vk::DescriptorBufferInfo>& GetGlobalBufferDescriptorInfo() const;  // per frame in flight
    const std::vector<vk::DescriptorBufferInfo>& GetLightBufferDescriptorInfo() const;
    const std::vector<vk::DescriptorBufferInfo>& GetPerObjectDescriptorBufferInfo(int meshIndex) const;  // per object per frame in flight
    std::vector<vk::DescriptorImageInfo>  GetAll2DTextureDescriptorImageInfo(const ApplicationCore::SceneData& sceneData) const;  // per object per frame in flight
    vk::DescriptorBufferInfo GetSceneBufferDescriptorInfo(int frameIndex) const;

    void UpdatePerFrameUniformData(int frameIndex, GlobalUniform& perFrameData) const;

    void UpdatePerObjectUniformData(int frameIndex, std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& drawCalls) const;

    void UpdateLightUniformData(int frameIndex, LightStructs::SceneLightInfo& sceneLightInfo) const;

    void UpdateSceneDataInfo(int frameIndex, const ApplicationCore::SceneData& sceneData) const;

    void Destroy() const;

  private:
    void CreateUniforms();

  private:
    const VulkanCore::VDevice& m_device;

    std::unique_ptr<VUniform<LightUniforms>>                               m_lightUniform;
    std::unique_ptr<VulkanUtils::VUniform<GlobalUniform>>                  m_perFrameUniform;

    std::vector<std::unique_ptr<VulkanUtils::VUniform<ObjectDataUniform>>> m_perObjectUniform;

    //=======================================================
    // storage buffers containing all of the data for materials
    std::vector<std::unique_ptr<VulkanCore::VShaderStorageBuffer>> m_sceneMaterials;
    std::vector<std::unique_ptr<VulkanCore::VShaderStorageBuffer>> m_perObjectData;

    mutable int m_currentDrawCalls = 0;
};
}  // namespace VulkanUtils


#endif  //VUNIFORMBUFFERMANAGER_HPP
