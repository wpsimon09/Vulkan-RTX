//
// Created by wpsimon09 on 17/09/24.
//

#include "Application.h"

#include <memory>

#include "WindowManager/WindowManager.h"


void Application::Init()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
}

void Application::Run()
{
    m_windowManager->InitWindow();
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
