//
// Created by wpsimon09 on 20/03/25.
//

#ifndef EFFECTSLIBRARY_HPP
#define EFFECTSLIBRARY_HPP
#include <map>
#include <memory>
#include <glm/fwd.hpp>


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
    EditorBilboard
};

class EffectsLibrary
{
  public:
    EffectsLibrary(const VulkanCore::VDevice& device, VulkanUtils::VResourceGroupManager& pushDescriptorManager);
    std::map<EEffectType, std::shared_ptr<VulkanUtils::VRasterEffect>> effects;

    std::shared_ptr<VulkanUtils::VRasterEffect> GetEffect(EEffectType type);

    void BuildAllEffects();
    void Destroy();
};

}  // namespace ApplicationCore

#endif  //EFFECTSLIBRARY_HPP
