//
// Created by wpsimon09 on 16/08/2025.
//

#include "RenderPass.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"



namespace Renderer {
RenderPass::RenderPass(const VulkanCore::VDevice& device, int width, int height)
  : m_device(device), m_width(width), m_height(height)
{

}
void RenderPass::Destroy() {
  for (auto& renderTarget : m_renderTargets) {
      renderTarget.Destroy();
  }
}
}  // namespace Renderer