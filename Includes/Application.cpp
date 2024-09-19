//
// Created by wpsimon09 on 17/09/24.
//


#include "Application.hpp"

#include <memory>
#include <stdexcept>

#include "WindowManager/WindowManager.hpp"
import VDevices;

Application::Application()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
}

void Application::Init()
{
    m_windowManager->InitWindow();
    InitVulkan();
}

void Application::Run()
{
    Init();
    MainLoop();
}

void Application::Update()
{
}

void Application::Render()
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
