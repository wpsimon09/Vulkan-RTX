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

namespace VEditor
{
    class Editor;
}

namespace VEditor
{
    class UIContext;
}

namespace Renderer
{
    class RenderingSystem;
}

namespace VulkanUtils
{
    class VPushDescriptorManager;
    class VUniformBufferManager;
}

class Client;

namespace VulkanCore
{
    class VBufferAllocator;
    class VCommandPool;
    class VFrameBuffer;
    class VRenderPass;
    class VPipelineManager;
    class VGraphicsPipeline;
    class VSwapChain;
    class VDevice;
    class VulkanInstance;

}

class Application {
public:
    Application();

    void Init();

    void Run();

    void Update();

    void Render();

    void MainLoop();

    ~Application();

private:
    std::unique_ptr<class Client> m_client;

    std::unique_ptr<class WindowManager> m_windowManager;
    std::unique_ptr<class VulkanUtils::VPushDescriptorManager> m_pushDescriptorSetManager;
    std::unique_ptr<class VulkanUtils::VUniformBufferManager> m_uniformBufferManager;
    std::unique_ptr<class VulkanCore::VulkanInstance> m_vulkanInstance;
    std::unique_ptr<class VulkanCore::VDevice> m_vulkanDevice;
    std::unique_ptr<VulkanCore::VBufferAllocator> m_bufferAllocator;
    std::unique_ptr<class VEditor::UIContext> m_uiContext;
    std::unique_ptr<class VEditor::Editor> m_editor;
    std::unique_ptr<class Renderer::RenderingSystem> m_renderingSystem;
};

#endif //VULKANRTX_HPP
