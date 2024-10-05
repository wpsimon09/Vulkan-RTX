//
// Created by wpsimon09 on 22/09/24.
//

#include "VulkanRtx.hpp"
#include <GLFW/glfw3.h>
#include <chrono>

#include "Application/Client.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"


Application::Application()
{
}

void Application::Init()
{
    m_windowManager = std::make_unique<WindowManager>(800,600);
    m_windowManager->InitWindow();

    m_client = std::make_unique<Client>();
    m_client->Init();

    m_vulkanInstance = std::make_unique<VulkanCore::VulkanInstance>("Vulkan-RTX", m_windowManager->GetWindow());
    m_vulkanDevice = std::make_unique<VulkanCore::VDevice>(*m_vulkanInstance);
    m_swapChain = std::make_unique<VulkanCore::VSwapChain>(*m_vulkanDevice, *m_vulkanInstance);

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
    auto start = std::chrono::high_resolution_clock::now();

    Init();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    Utils::Logger::LogInfo("Application took: " + std::to_string(duration.count()) + " milliseconds to set-up");
    Utils::Logger::LogSuccess("APPLICATION CONFIGURED SUCCESSFULLY !");

    MainLoop();
}

void Application::Update()
{
}

Application::~Application() {
    m_swapChain->Destroy();
    m_vulkanDevice->Destroy();
}
