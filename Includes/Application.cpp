//
// Created by wpsimon09 on 17/09/24.
//

#include "Application.h"

#include <memory>
#include <stdexcept>

#include "WindowManager/WindowManager.h"


Application::Application()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
}

void Application::Init()
{
    m_windowManager->InitWindow();
}

void Application::Run()
{
    Init();
    throw std::runtime_error("Test");
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
