//
// Created by wpsimon09 on 20/12/24.
//

#ifndef IMGUIINITIALIZER_HPP
#define IMGUIINITIALIZER_HPP

#include <GLFW/glfw3.h>
#include <ImGuizmo/ImGuizmo.h>

#include "ViewPortContext.hpp"
#include "Application/Client.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace ApplicationCore {
class SceneNode;
}

class Client;

namespace Renderer {
class RenderingSystem;
}

namespace ApplicationCore {
class Scene;
}

struct ImGuiIO;

namespace VulkanCore {
class VRenderPass;
class VulkanInstance;
class VDevice;
}  // namespace VulkanCore

namespace VEditor {
class Editor;
class UIContext
{
  public:
    explicit UIContext(const VulkanCore::VDevice&        device,
                       const VulkanCore::VulkanInstance& instance,
                       WindowManager&                    windowManager,
                       Client&                           client);

    ViewPortContext&           GetViewPortContext(ViewPortType viewPortType) { return m_viewports[viewPortType]; }
    const VulkanCore::VDevice& GetDevice() { return m_device; }

    void Initialize(const VulkanCore::VSwapChain& swapChain);
    void BeginRender();
    void Render(VulkanCore::VCommandBuffer& commandBuffer);
    void SetRenderingSystem(Renderer::RenderingSystem* rendderingSystem) { m_renderingSystem = rendderingSystem; }
    void EndRender();
    void Destroy();
    void SetCurrentOperation(ImGuizmo::OPERATION operation) { m_operation = operation; }
    std::unordered_map<ViewPortType, ViewPortContext>&  GetViewPorts() {return m_viewports;};

    ;
    void SetSelectedSceneNode(std::shared_ptr<ApplicationCore::SceneNode> sceneNode)
    {
        m_selectedSceneNode = sceneNode;
    };
    void SetColourThemePabloDark();
    void SetColourThemePabloLight();

    bool m_isRayTracing = false;


  public:
    ApplicationCore::Scene& GetScene() const { return m_client.GetScene(); }
    Client&                 GetClient() { return m_client; }

  private:
    const VulkanCore::VDevice&        m_device;
    const VulkanCore::VulkanInstance& m_instance;
    WindowManager&                    m_windowManager;
    Client&                           m_client;

    Renderer::RenderingSystem* m_renderingSystem;

    vk::PipelineCache  m_imguiPipelineCache;
    vk::DescriptorPool m_imguiDescriptorPool;

    ImDrawData* m_imguiDrawData;

    std::unordered_map<ViewPortType, ViewPortContext> m_viewports;

    ImGuiIO* m_io;

    ImFont* m_defaultFont;
    ImFont* m_editorFont;

    std::shared_ptr<ApplicationCore::SceneNode> m_selectedSceneNode = nullptr;

    ImGuizmo::OPERATION m_operation;


  private:
    ;

  private:
    friend class VEditor::Editor;
};

}  // namespace VEditor

#endif  //IMGUIINITIALIZER_HPP
