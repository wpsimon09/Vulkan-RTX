//
// Created by wpsimon09 on 20/03/25.
//

#ifndef EFFECTSLIBRARY_HPP
#define EFFECTSLIBRARY_HPP
#include <map>
#include <memory>
#include <glm/fwd.hpp>
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"


namespace VulkanStructs {
struct RenderContext;
}
namespace VulkanUtils {
struct RenderContext;
class VRayTracingDataManager;
class VEffect;
}
namespace VulkanUtils {
class VRasterEffect;
class VResourceGroupManager;
}  // namespace VulkanUtils

namespace Renderer {
class RenderingSystem;
}

namespace VulkanCore {
class VDevice;
}

namespace ApplicationCore {

enum class EEffectType : std::uint8_t
{
    Outline = 0,
    ForwardShader,
    SkyBox,
    DebugLine,
    AlphaMask,
    AplhaBlend,
    EditorBilboard,
    RayTracing,
    DepthPrePass,
    RTShadowPass
};

class EffectsLibrary
{
  public:
    EffectsLibrary(const VulkanCore::VDevice&          device,
                   VulkanUtils::VUniformBufferManager& uniformBufferManager,
                   VulkanUtils::VRayTracingDataManager& rtxDataManager,
                   VulkanCore::VDescriptorLayoutCache& descLayoutCache);


    std::map<EEffectType, std::shared_ptr<VulkanUtils::VEffect>> effects;
    std::shared_ptr<VulkanUtils::VEffect> GetEffect(EEffectType type);

    template <typename T>
    std::shared_ptr<T> GetEffect(EEffectType type);

    void BuildAllEffects();
    void Destroy();

    void UpdatePerFrameWrites(const Renderer::SceneRenderer& sceneRenderer,  VulkanUtils::RenderContext* renderingContext,const VulkanUtils::VUniformBufferManager& uniformBufferManager);

    void ConfigureDescriptorWrites(const Renderer::SceneRenderer& sceneRenderer, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::VRayTracingDataManager& rayTracingDataManager);
  private:

    VulkanCore::VDescriptorLayoutCache& m_descLayoutCache;
};
template <typename T>
std::shared_ptr<T> EffectsLibrary::GetEffect(EEffectType type)
{
    assert(effects.contains(type));
    return std::dynamic_pointer_cast<T>(effects[type]);
}

}  // namespace ApplicationCore

#endif  //EFFECTSLIBRARY_HPP
