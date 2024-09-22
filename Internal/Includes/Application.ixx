//
// Created by wpsimon09 on 19/09/24.
//
module;

#include <memory>
#include "GLFW/glfw3.h"

export module Application;

import Window;
import VDevices;

export class Application {
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
};

Application::Application()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
}

void Application::Init()
{
    m_windowManager->InitWindow();
    InitVulkan();
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
