//
// Created by wpsimon09 on 28/09/2025.
//

#ifndef VULKAN_RTX_ATMOSPHEREPASS_HPP
#define VULKAN_RTX_ATMOSPHEREPASS_HPP
#include "Application/Structs/ParameterStructs.hpp"
#include "RenderPass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"

namespace VulkanStructs {
struct PostProcessingContext;
}
namespace VulkanUtils {
struct RenderContext;
class VUniformBufferManager;
class VComputeEffect;
}  // namespace VulkanUtils
namespace ApplicationCore {
class EffectsLibrary;
}
namespace VulkanCore {
class VCommandBuffer;
class VImage2;
}  // namespace VulkanCore


namespace Renderer {

enum EAtmosphereAttachments
{
    TransmitanceLUT = 0,
    MultipleScatteringLut,
    SkyViewLut,
    AtmosphereComposition
};

class AtmospherePass : public Renderer::RenderPass
{
  public:
    AtmospherePass(const VulkanCore::VDevice& device, ApplicationCore::EffectsLibrary& effectsLibrary, int width, int height);
    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;
    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;
    void Precompute(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext);

    void Destroy() override;

  private:
    std::shared_ptr<VulkanUtils::VComputeEffect> m_transmitanceLutEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_multipleScatteringLutEffect;
    std::shared_ptr<VulkanUtils::VComputeEffect> m_skyViewLutEffect;
    // final composition of the atmosphere
    std::shared_ptr<VulkanUtils::VRasterEffect> m_atmospherePassEffect;


    AtmosphereParameters m_atmosphereParams;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_ATMOSPHEREPASS_HPP
