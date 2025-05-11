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
#include "Application/Rendering/Mesh/StaticMesh.hpp"
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
#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Application/GLTFExporter/GLTFExporter.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Utils/VEffect/VRayTracingEffect.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"
#include "Vulkan/Renderer/Renderers/RayTracing/RayTracer.hpp"
#include "Vulkan/VulkanCore/Buffer/VShaderStorageBuffer.hpp"

// Vulkan Utilities
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"

// Application Entry
#include "VulkanRtx.hpp"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/SceneRenderer.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"


// Im gui entry
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/GLTFLoader/LoadSceneLights.hpp"
#include "Application/Utils/LinearyTransformedCosinesValues.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Application/Utils/Parsers/EngineDataParser.hpp"
#include "Editor/Editor.hpp"
#include "Editor/UIContext/UIContext.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"

#include "Vulkan/Utils/VEnvLightGenerator/VEnvLightGenerator.hpp"
#include "Vulkan/Utils/VRayTracingManager/VRayTracingDataManager.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhrHelpers.hpp"

Application::Application() {}

void Application::Init()
{
    ApplicationCore::LoadConfig();

    m_client = std::make_unique<Client>();

    m_windowManager = std::make_unique<WindowManager>(1000, 800);
    m_windowManager->InitWindow();

    m_vulkanInstance = std::make_unique<VulkanCore::VulkanInstance>("Vulkan-RTX", m_windowManager->GetWindow());
    m_vulkanDevice   = std::make_unique<VulkanCore::VDevice>(*m_vulkanInstance);

    m_vulkanDevice->GetTransferOpsManager().StartRecording();

    VulkanCore::VSamplers::CreateAllSamplers(*m_vulkanDevice);
    MathUtils::InitLTC();

    m_descriptorSetLayoutCache = std::make_unique<VulkanCore::VDescriptorLayoutCache>(*m_vulkanDevice);

    m_pushDescriptorSetManager = std::make_unique<VulkanUtils::VResourceGroupManager>(*m_vulkanDevice);
    m_effectsLibrary = std::make_unique<ApplicationCore::EffectsLibrary>(*m_vulkanDevice, *m_pushDescriptorSetManager);
    auto assetManger = std::make_unique<ApplicationCore::AssetsManager>(*m_vulkanDevice, *m_effectsLibrary);
    m_client->MountAssetsManger(std::move(assetManger));
    m_client->Init();

    m_uniformBufferManager = std::make_unique<VulkanUtils::VUniformBufferManager>(*m_vulkanDevice);

    //m_renderer = std::make_unique<Renderer::VRenderer>(*m_vulkanInstance, *m_vulkanDevice, *m_uniformBufferManager, *m_resrouceGroupManager);
    m_uiContext = std::make_unique<VEditor::UIContext>(*m_vulkanDevice, *m_vulkanInstance, *m_windowManager, *m_client);

    m_renderingSystem = std::make_unique<Renderer::RenderingSystem>(*m_vulkanInstance, *m_vulkanDevice, *m_uniformBufferManager,

                                                                    *m_pushDescriptorSetManager, *m_uiContext);


    m_renderingSystem->Init();
    m_uiContext->SetRenderingSystem(m_renderingSystem.get());


    //auto sponsa = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Desktop/Models/sponza_scene/scene.gltf");
    ApplicationCore::ImportOptions importOptions{};

    m_client->GetGLTFLoader().LoadGLTFScene(m_client->GetScene(),"cache/scene.gltf", importOptions);

    m_editor = std::make_unique<VEditor::Editor>(*m_uiContext);

    ApplicationCore::LoadClientSideConfig(*m_client, *m_uiContext);

    m_vulkanDevice->GetTransferOpsManager().UpdateGPUWaitCPU(true);
    m_client->GetScene().Update();
    auto inputs =m_client->GetScene().GetBLASInputs();
    m_renderingSystem->GetRayTracingManager().InitAs(inputs);
}

void Application::MainLoop()
{

    while(!glfwWindowShouldClose(m_windowManager->GetWindow()))
    {
        Update();
        Render();
        PostRender();

        glfwPollEvents();
    }
}

void Application::Run()
{
    auto start = std::chrono::high_resolution_clock::now();

    Init();

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    Utils::Logger::LogInfo("Application took: " + std::to_string(duration.count()) + " milliseconds to set-up");
    Utils::Logger::LogSuccess("APPLICATION CONFIGURED SUCCESSFULLY !");


    MainLoop();
}

void Application::Update()
{
    m_vulkanDevice->GetTransferOpsManager().StartRecording();

    m_client->Update();
    m_client->UpdateCamera(m_windowManager->GetCameraMovement());
    if(m_windowManager->GetIsDirty())
    {
        m_client->UpdateClient(m_windowManager->GetLightMovement());
    }
    if(GlobalState::ValidationLayersEnabled)
    {
        m_vulkanDevice->UpdateMemoryStatistics();
    }

    m_editor->SetVmaStatis(m_vulkanDevice->GetDeviceStatistics());
    m_editor->Update();

    if (m_client->GetScene().GetSceneUpdateFlags().updateAs) {
        auto blasInput = m_client->GetScene().GetBLASInputs();
        m_renderingSystem->GetRayTracingManager().UpdateAS(blasInput);
        Utils::Logger::LogInfo("Updating AS");
    }

    if (m_client->GetScene().GetSceneUpdateFlags().rebuildAs
        ) {
        auto blasInpu = m_client->GetScene().GetBLASInputs();
        m_renderingSystem->GetRayTracingManager().InitAs(blasInpu);
        Utils::Logger::LogInfo("Rebuilding AS");
    }

}

void Application::Render()
{


    m_client->GetAssetsManager().Sync();

    m_client->Render(m_renderingSystem->GetRenderContext());

    m_editor->Render();

    m_renderingSystem->Render(m_client->GetScene().GetSceneLightInfo(),m_client->GetScene().GetSceneData(), m_client->GetGlobalDataUpdateInformation(), m_client->GetScene().GetSceneUpdateFlags());

    m_renderingSystem->Update();
}

void Application::PostRender()
{
    m_vulkanDevice->GetTransferOpsManager().ClearResources();
    m_client->GetScene().Reset();
}

Application::~Application()
{
    ApplicationCore::SaveConfig(*m_client, *m_uiContext);

    if(!GlobalVariables::hasSessionBeenSaved)
    {
        for(const auto& entry : std::filesystem::directory_iterator(GlobalVariables::textureFolder))
        {
            std::filesystem::remove_all(entry.path());
        }
    }
    m_vulkanDevice->GetDevice().waitIdle();
    m_pushDescriptorSetManager->Destroy();
    m_renderingSystem->Destroy();
    m_effectsLibrary->Destroy();
    m_client->Destroy();
    m_uniformBufferManager->Destroy();
    VulkanCore::VSamplers::DestroyAllSamplers(*m_vulkanDevice);
    m_uiContext->Destroy();
    MathUtils::LUT.ClearLoopUpTables();

    m_vulkanDevice->Destroy();
}
