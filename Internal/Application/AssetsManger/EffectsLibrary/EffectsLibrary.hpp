//
// Created by wpsimon09 on 20/03/25.
//

#ifndef EFFECTSLIBRARY_HPP
#define EFFECTSLIBRARY_HPP
#include <map>
#include <memory>
#include <glm/fwd.hpp>


namespace VulkanUtils {
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
    EditorBilboard
};

class EffectsLibrary
{
  public:
    EffectsLibrary(const VulkanCore::VDevice& device, VulkanUtils::VResourceGroupManager& pushDescriptorManager);
    std::map<EEffectType, std::shared_ptr<VulkanUtils::VEffect>> effects;

    std::shared_ptr<VulkanUtils::VEffect> GetEffect(EEffectType type);

    template <typename T>
    std::shared_ptr<T> GetEffect(EEffectType type);

    void BuildAllEffects();
    void Destroy();
};
template <typename T>
std::shared_ptr<T> EffectsLibrary::GetEffect(EEffectType type)
{
  assert(effects.contains(type));
  return std::dynamic_pointer_cast<T>(effects[type]);
}

}  // namespace ApplicationCore

#endif  //EFFECTSLIBRARY_HPP
