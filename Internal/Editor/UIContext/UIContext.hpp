//
// Created by wpsimon09 on 20/12/24.
//

#ifndef IMGUIINITIALIZER_HPP
#define IMGUIINITIALIZER_HPP

#include <GLFW/glfw3.h>

#include "ViewPortContext.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

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
        const WindowManager& windowManager,
        const ApplicationCore::Scene& scene
    );

    ViewPortContext& GetViewPortContext(ViewPortType viewPortType) {return m_viewports[viewPortType];};

    void Initialize(const VulkanCore::VRenderPass& renderPass);
    void BeginRender();
    void Render(VulkanCore::VCommandBuffer& commandBuffer);
    void EndRender();
    void Destroy();
private:
    const ApplicationCore::Scene& m_scene;
    const VulkanCore::VDevice& m_device;
    const VulkanCore::VulkanInstance& m_instance;
    const WindowManager& m_windowManager;

    vk::PipelineCache m_imguiPipelineCache;
    vk::DescriptorPool m_imguiDescriptorPool;

    ImDrawData* m_imguiDrawData;

    std::unordered_map<ViewPortType, ViewPortContext> m_viewports;

    ImGuiIO* m_io;

    ImFont* m_defaultFont;
    ImFont* m_editorFont;

    private:
        void ThemeConfig();

    friend class VEditor::Editor;
};

} // VEditor

#endif //IMGUIINITIALIZER_HPP
