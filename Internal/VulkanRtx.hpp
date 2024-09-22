//
// Created by wpsimon09 on 22/09/24.
//

#ifndef VULKANRTX_HPP
#define VULKANRTX_HPP

#include <memory>
#include "Includes/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/VInstance.hpp"


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
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<VulkanCore::VulkanInstance> m_vulkanInstance;
};

#endif //VULKANRTX_HPP
