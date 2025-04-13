//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VULKANRTX_HPP
#define VULKANRTX_HPP

#include <memory>
#include <vector>
#include <VMA/vk_mem_alloc.h>

#include "Editor/Views/Index.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"

namespace VulkanCore {
namespace RTX {
class VRayTracingBuilderKHR;
}

class VRayTracingBuilderKHR;
}  // namespace VulkanCore

namespace ApplicationCore {
class EffectsLibrary;
}

namespace VulkanUtils {
class VTransferOperationsManager;
}

namespace VEditor {
class Editor;
}

namespace VEditor {
class UIContext;
}

namespace Renderer {
class RenderingSystem;
}

namespace VulkanUtils {
class VResourceGroupManager;
class VUniformBufferManager;
}  // namespace VulkanUtils

class Client;

namespace VulkanCore {
class MeshDatatManager;
class VCommandPool;
class VFrameBuffer;
class VRenderPass;
class VPipelineManager;
class VGraphicsPipeline;
class VSwapChain;
class VDevice;
class VulkanInstance;

}  // namespace VulkanCore

class Application
{
public:
  Application();

  void Init();

  void Run();

  void Update();

  void Render();

  void PostRender();

  void MainLoop();

  ~Application();

private:
  std::unique_ptr<class Client> m_client;

  std::unique_ptr<class WindowManager>                          m_windowManager;
  std::unique_ptr<class VulkanUtils::VResourceGroupManager>     m_pushDescriptorSetManager;
  std::unique_ptr<class VulkanUtils::VUniformBufferManager>     m_uniformBufferManager;
  std::unique_ptr<class VulkanCore::VulkanInstance>             m_vulkanInstance;
  std::unique_ptr<class VulkanCore::VDevice>                    m_vulkanDevice;
  std::unique_ptr<class VEditor::UIContext>                     m_uiContext;
  std::unique_ptr<class VEditor::Editor>                        m_editor;
  std::unique_ptr<class Renderer::RenderingSystem>              m_renderingSystem;
  std::unique_ptr<class ApplicationCore::EffectsLibrary>        m_effectsLibrary;
  std::unique_ptr<class VulkanCore::RTX::VRayTracingBuilderKHR> m_rayTracingBuilder;
};

#endif  //VULKANRTX_HPP
