//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VULKANRTX_HPP
#define VULKANRTX_HPP

#include <memory>


namespace VulkanCore
{
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
    inline static Application* instance = nullptr;

    std::unique_ptr<class WindowManager> m_windowManager;
    std::unique_ptr<class VulkanCore::VulkanInstance> m_vulkanInstance;
    std::unique_ptr<class VulkanCore::VDevice> m_vulkanDevice;
    std::unique_ptr<VulkanCore::VSwapChain> m_swapChain;
};

#endif //VULKANRTX_HPP
