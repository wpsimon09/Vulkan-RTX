//
// Created by wpsimon09 on 22/09/24.
//

    #include "VulkanRtx.hpp"
    #include <GLFW/glfw3.h>
    #include <chrono>

    // Application Headers
    #include "Application/Logger/Logger.hpp"
    #include "Application/WindowManager/WindowManager.hpp"
    #include "Application/Client.hpp"
    #include "Application/AssetsManger/AssetsManager.hpp"
    #include "Application/Rendering/Mesh/Mesh.hpp"
    #include "Application/VertexArray/VertexArray.hpp"
    #include "Application/Rendering/Camera/Camera.hpp"
    #include "Application/Rendering/Transformations/Transformations.hpp"
    #include "Application/Rendering/Scene/Scene.hpp"
    #include "Application/Rendering/Scene/SceneNode.hpp"

    // Vulkan Core
    #include "Vulkan/VulkanCore/Instance/VInstance.hpp"
    #include "Vulkan/VulkanCore/Device/VDevice.hpp"
    #include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
    #include "Vulkan/VulkanCore/FrameBuffer/VFrameBuffer.hpp"
    #include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
    #include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
    #include "Vulkan/VulkanCore/Shader/VShader.hpp"
    #include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
    #include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
    #include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
    #include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
    #include "Vulkan/VulkanCore/Pipeline/VPipelineManager.hpp"
    #include "Application/GLTFLoader/GltfLoader.hpp"
    #include "Vulkan/VulkanCore/VImage/VImage.hpp"

    // Vulkan Utilities
    #include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"

    // Vulkan Renderer
    #include "Vulkan/Renderer/VRenderer.hpp"

    // Application Entry
    #include "VulkanRtx.hpp"

#include "Vulkan/Utils/VImGuiInitializer/ImGuiInitializer.hpp"


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

    VulkanCore::VSamplers::CreateAllSamplers(*m_vulkanDevice);

    auto assetManger = std::make_unique<ApplicationCore::AssetsManager>(*m_vulkanDevice);
    m_client->MountAssetsManger(std::move(assetManger));
    m_client->Init();

    m_pushDescriptorSetManager = std::make_unique<VulkanUtils::VPushDescriptorManager>(*m_vulkanDevice);
    m_uniformBufferManager = std::make_unique<VulkanUtils::VUniformBufferManager>(*m_vulkanDevice);
    m_renderer = std::make_unique<Renderer::VRenderer>(*m_vulkanInstance, *m_vulkanDevice, *m_uniformBufferManager, *m_pushDescriptorSetManager);


    m_imguiInitializer = std::make_unique<VulkanUtils::ImGuiInitializer>(*m_vulkanDevice, *m_vulkanInstance, m_renderer->GetRenderPass(), *m_windowManager);
    m_imguiInitializer->Initialize();

    //auto sponsa = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Desktop/Models/sponza_scene/scene.gltf");
    auto sponsa = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Downloads/sponza_scene.glb");
    m_client->GetScene().AddNode(sponsa);

    //auto car = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Desktop/Models/dodge_challenger_-_muscle_car_-_low-poly/scene.gltf");
    //auto stormTrooper = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Desktop/Models/storm-trooper.glb");
    auto stormTrooper = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Downloads/star_wars_at-st.glb");
    m_client->GetScene().AddNode(stormTrooper);
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
    m_client->Update();
    if(m_windowManager->GetIsDirty()) {
    m_client->UpdateCamera(m_windowManager->GetCameraMovement());
    m_client->UpdateClient(m_windowManager->GetLightMovement());
    }
}

void Application::Render() {
    m_client->GetAssetsManager().Sync();
    m_client->Render(m_renderer->GetRenderingContext()); // here
    m_renderer->SetRtxStatus(m_client->GetIsRTXOn());
    m_renderer->Render(m_client->GetGlobalDataUpdateInformation());
}

Application::~Application() {
    m_vulkanDevice->GetDevice().waitIdle();
    m_renderer->Destroy();
    m_client->Destroy();
    m_uniformBufferManager->Destroy();
    VulkanCore::VSamplers::DestroyAllSamplers(*m_vulkanDevice);
    m_pushDescriptorSetManager->Destroy();
    m_vulkanDevice->Destroy();
}
