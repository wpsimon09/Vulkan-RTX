//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VULKANRTX_HPP
#define VULKANRTX_HPP

#include <memory>
#include "Includes/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

class Application {
public:
    Application();

    void Init();

    void Run();

    void Update();

    void Render(){};

    void MainLoop();

    ~Application() = default;

private:
    inline static Application* instance = nullptr;

    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<VulkanCore::VulkanInstance> m_vulkanInstance;
    std::unique_ptr<VulkanCore::VDevice> m_vulkanDevice;
};

#endif //VULKANRTX_HPP
