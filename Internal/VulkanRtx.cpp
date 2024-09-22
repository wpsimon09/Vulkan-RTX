//
// Created by wpsimon09 on 22/09/24.
//

#include "VulkanRtx.hpp"

#include <GLFW/glfw3.h>

#include "Includes/WindowManager/WindowManager.hpp"


Application::Application()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
    m_vulkanInstance = std::make_unique<VulkanCore::VulkanInstance>("Vulkan-RTX");
}

void Application::Init()
{

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
