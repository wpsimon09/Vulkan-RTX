//
// Created by wpsimon09 on 22/09/24.
//

#include "VulkanRtx.hpp"
#include <GLFW/glfw3.h>


Application::Application()
{
}

void Application::Init()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
    m_windowManager->InitWindow();

    m_vulkanInstance = std::make_unique<VulkanCore::VulkanInstance>("Vulkan-RTX");
}

void Application::MainLoop()
{
    while(!glfwWindowShouldClose(m_windowManager->GetWindow()))
    {
        Update();
        Render();
        glfwPollEvents();
    }
}

void Application::Run()
{
    Init();
    MainLoop();
}

void Application::Update()
{
}
