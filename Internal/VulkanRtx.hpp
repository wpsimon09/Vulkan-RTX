//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VULKANRTX_HPP
#define VULKANRTX_HPP

#include <memory>
#include <vector>

#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"

namespace Renderer
{
    class VRenderer;
}

namespace VulkanCore
{
    class VCommandPool;
}

class Client;

namespace VulkanCore
{
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

    ~Application() ;

private:
    std::unique_ptr<class Client> m_client;

    std::unique_ptr<class WindowManager> m_windowManager;
    std::unique_ptr<class VulkanCore::VulkanInstance> m_vulkanInstance;
    std::unique_ptr<class VulkanCore::VDevice> m_vulkanDevice;
    std::unique_ptr<class Renderer::VRenderer> m_renderer;
};

#endif //VULKANRTX_HPP
