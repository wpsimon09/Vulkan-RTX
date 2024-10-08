//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VULKANRTX_HPP
#define VULKANRTX_HPP

#include <memory>

class Client;

namespace VulkanCore
{
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

    void Render(){};

    void MainLoop();

    ~Application() ;

private:
    std::unique_ptr<class Client> m_client;

    std::unique_ptr<class WindowManager> m_windowManager;
    std::unique_ptr<class VulkanCore::VulkanInstance> m_vulkanInstance;
    std::unique_ptr<class VulkanCore::VDevice> m_vulkanDevice;
    std::unique_ptr<class VulkanCore::VSwapChain> m_swapChain;
    std::unique_ptr<class VulkanCore::VPipelineManager> m_pipelineManager;
    std::unique_ptr<class VulkanCore::VRenderPass> m_mainRenderPass;
};

#endif //VULKANRTX_HPP
