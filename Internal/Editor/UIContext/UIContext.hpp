//
// Created by wpsimon09 on 20/12/24.
//

#ifndef IMGUIINITIALIZER_HPP
#define IMGUIINITIALIZER_HPP

#include <GLFW/glfw3.h>

#include "ViewPortContext.hpp"
#include "Application/Client.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

class Client;

namespace Renderer
{
    class RenderingSystem;
}

namespace ApplicationCore
{
    class Scene;
}

struct ImGuiIO;

namespace VulkanCore
{
    class VRenderPass;
    class VulkanInstance;
    class VDevice;
}

namespace VEditor {
    class Editor;
    class UIContext {
public:
    explicit UIContext(
        const VulkanCore::VDevice& device,
        const VulkanCore::VulkanInstance& instance,
        WindowManager& windowManager,
        const Client& client
    );

    ViewPortContext& GetViewPortContext(ViewPortType viewPortType) {return m_viewports[viewPortType];};

    void Initialize(const VulkanCore::VRenderPass& renderPass);
    void BeginRender();
    void Render(VulkanCore::VCommandBuffer& commandBuffer);
    void SetRenderingSystem(Renderer::RenderingSystem* rendderingSystem) {m_renderingSystem = rendderingSystem;}
    void EndRender();
    void Destroy();
public:
    const ApplicationCore::Scene& GetScene() const {return m_client.GetScene();}
    const Client& GetClient() const {return m_client;}
private:
    const VulkanCore::VDevice& m_device;
    const VulkanCore::VulkanInstance& m_instance;
    WindowManager& m_windowManager;
    const Client& m_client;

    Renderer::RenderingSystem* m_renderingSystem;

    vk::PipelineCache m_imguiPipelineCache;
    vk::DescriptorPool m_imguiDescriptorPool;

    ImDrawData* m_imguiDrawData;

    std::unordered_map<ViewPortType, ViewPortContext> m_viewports;

    ImGuiIO* m_io;

    ImFont* m_defaultFont;
    ImFont* m_editorFont;

    private:
        void SetColourThemePablo();

    friend class VEditor::Editor;
};

} // VEditor

#endif //IMGUIINITIALIZER_HPP
