//
// Created by wpsimon09 on 22/09/24.
//

#include "VulkanRtx.hpp"
#include <GLFW/glfw3.h>
#include <chrono>

#include "Application/Logger/Logger.hpp"
#include "Application/WindowManager/WindowManager.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Application/Client.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Renderer/VRenderer.hpp"
#include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Utils/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"


Application::Application()
{
}

void Application::Init()
{
    m_client = std::make_unique<Client>();

    m_windowManager = std::make_unique<WindowManager>(800,600);
    m_windowManager->InitWindow();

    m_vulkanInstance = std::make_unique<VulkanCore::VulkanInstance>("Vulkan-RTX", m_windowManager->GetWindow());
    m_vulkanDevice = std::make_unique<VulkanCore::VDevice>(*m_vulkanInstance);


    auto assetManger = std::make_unique<ApplicationCore::AssetsManager>(*m_vulkanDevice);
    m_client->MountAssetsManger(std::move(assetManger));
    m_client->Init();

    m_pushDescriptorSetManager = std::make_unique<VulkanUtils::VPushDescriptorManager>(*m_vulkanDevice);
    m_uniformBufferManager = std::make_unique<VulkanUtils::VUniformBufferManager>(*m_vulkanDevice, *m_client);
    m_renderer = std::make_unique<Renderer::VRenderer>(*m_vulkanInstance, *m_vulkanDevice, *m_client, *m_uniformBufferManager, *m_pushDescriptorSetManager);

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
    if(m_windowManager->GetIsDirty()) {
        m_client->Update(m_windowManager->GetCameraMovement());
    }
}

void Application::Render() {
    m_renderer->Render();
}

Application::~Application() {
    m_vulkanDevice->GetDevice().waitIdle();
    m_renderer->Destroy();
    m_client->Destroy();
    m_uniformBufferManager->Destroy();
    m_vulkanDevice->Destroy();
}
