//
// Created by wpsimon09 on 18/08/2025.
//

#ifndef VULKAN_RTX_LIGHTPASS_HPP
#define VULKAN_RTX_LIGHTPASS_HPP

#include "Vulkan/Renderer/Renderers/RenderPass/RenderPass.hpp"
#include "unordered_map"

namespace ApplicationCore {
  class EffectsLibrary;
};

namespace VulkanStructs {
  struct PostProcessingContext;
};

namespace VulkanUtils {
  class VRasterEffect;
  class VUniformBufferManager;
  struct RenderContext;
};

namespace VulkanCore {
  class VDevice;
  class VCommandBuffer;
}

namespace Renderer {

enum EForwardRenderEffects {
     Outline = 0,
     ForwardShader,
     SkyBox,
     DebugLine,
     AlphaMask,
     AplhaBlend,
     EditorBilboard,
     WireFrame,

     // all new effects have to go above this
     ForwardRenderEffectsCount
};


enum EForwardRenderAttachments {
  Main = 0,
};

class ForwardRender : public Renderer::RenderPass
{
  public:
    ForwardRender(const VulkanCore::VDevice& device,ApplicationCore::EffectsLibrary& effectLibrary, int width, int height);

    void Init(int currentFrameIndex, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanUtils::RenderContext* renderContext) override;

    void Update(int                                   currentFrame,
                VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                VulkanUtils::RenderContext*           renderContext,
                VulkanStructs::PostProcessingContext* postProcessingContext) override;

    void Render(int currentFrame, VulkanCore::VCommandBuffer& cmdBuffer, VulkanUtils::RenderContext* renderContext) override;

  private:
    std::unordered_map<EForwardRenderEffects, std::shared_ptr<VulkanUtils::VRasterEffect>> m_effects;


};

}  // namespace Renderer

#endif  //VULKAN_RTX_LIGHTPASS_HPP
