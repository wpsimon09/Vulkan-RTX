//
// Created by wpsimon09 on 20/03/25.
//

#ifndef EFFECTSLIBRARY_HPP
#define EFFECTSLIBRARY_HPP
#include <map>
#include <glm/fwd.hpp>


namespace VulkanUtils
{
    class VEffect;
    class VPushDescriptorManager;
}

namespace Renderer
{
    class RenderingSystem;
}

namespace VulkanCore
{
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
        AplhaBlend
    };

class EffectsLibrary {
public:
    EffectsLibrary(const VulkanCore::VDevice& device,const Renderer::RenderingSystem& renderingSystem, const VulkanUtils::VPushDescriptorManager& pushDescriptorManager );
    std::map<EEffectType, VulkanUtils::VEffect> effects;
};

} // ApplicationCore

#endif //EFFECTSLIBRARY_HPP
