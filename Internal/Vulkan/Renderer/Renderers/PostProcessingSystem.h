//
// Created by wpsimon09 on 13/06/25.
//

#ifndef POSTPROCESSINGSYSTEM_H
#define POSTPROCESSINGSYSTEM_H
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {
class VImage2;
class VDevice;
}
namespace Renderer {
class SceneRenderer;

class PostProcessingSystem {
public:
  PostProcessingSystem(const VulkanCore::VDevice& device,ApplicationCore::EffectsLibrary& effectsLibrary,  VulkanUtils::VUniformBufferManager& uniformBufferManager, int width, int height);

  void Render(int frameIndex, VulkanCore::VCommandBuffer& commandBuffer,VulkanStructs::PostProcessingContext& postProcessingContext);
  VulkanCore::VImage2& GetRenderedResult();

  void Destroy();

private:
  void ToneMapping(int currentIndex, VulkanCore::VCommandBuffer& commandBuffer);

private:

  std::vector<std::unique_ptr<VulkanCore::VImage2>> m_toneMapResult;
  std::shared_ptr<VulkanUtils::VEffect> m_toneMappingEffect;

  const VulkanCore::VDevice& m_device;
  VulkanUtils::VUniformBufferManager& m_uniformBufferManager;

  int m_width, m_height;

};

} // Renderer

#endif //POSTPROCESSINGSYSTEM_H
