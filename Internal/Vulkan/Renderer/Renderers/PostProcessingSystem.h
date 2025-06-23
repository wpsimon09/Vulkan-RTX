//
// Created by wpsimon09 on 13/06/25.
//

#ifndef POSTPROCESSINGSYSTEM_H
#define POSTPROCESSINGSYSTEM_H
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace Renderer {
class RenderTarget2;
}
namespace VulkanCore {
class VImage2;
class VDevice;
}
namespace Renderer {
class ForwardRenderer;

class PostProcessingSystem {
public:
  PostProcessingSystem(const VulkanCore::VDevice& device,ApplicationCore::EffectsLibrary& effectsLibrary,  VulkanUtils::VUniformBufferManager& uniformBufferManager, int width, int height);

  void Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer,VulkanStructs::PostProcessingContext& postProcessingContext);
  VulkanCore::VImage2& GetRenderedResult(int frameIndex);

  void Destroy();

private:
  void ToneMapping(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer, VulkanStructs::PostProcessingContext& postProcessingContext);

private:

  std::shared_ptr<VulkanUtils::VEffect> m_toneMappingEffect;

  std::unique_ptr<RenderTarget2> m_toneMapOutput;

  const VulkanCore::VDevice& m_device;
  VulkanUtils::VUniformBufferManager& m_uniformBufferManager;

  int m_width, m_height;

};

} // Renderer

#endif //POSTPROCESSINGSYSTEM_H
