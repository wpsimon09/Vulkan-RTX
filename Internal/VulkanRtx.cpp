//
// Created by wpsimon09 on 22/09/24.
//

#include "VulkanRtx.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <exception>

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
#include "Vulkan/Renderer/Renderers/Frame.hpp"
#include "Vulkan/Renderer/Renderers/ForwardRenderer.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"


// Im gui entry
#include "Application/Rendering/Scene/AtmosphereSceneNode.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/GLTFLoader/LoadSceneLights.hpp"
#include "Application/Utils/LookUpTables.hpp"
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
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"
#include "Vulkan/Renderer/Renderers/ForwardRenderer.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget2.h"
#include "Vulkan/Renderer/Renderers/PostProcessingSystem.h"
#include "Application/ApplicationState/ApplicationState.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/VisibilityBufferPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/GBufferPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/DenoisePass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/LightPass.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/PostProcessing.hpp"
#include "Vulkan/VulkanCore/Buffer/VGrowableBuffer.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore2.hpp"
#include "Vulkan/Renderer/Renderers/RenderPass/AtmospherePass.hpp"

Application::Application() = default;

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
    MathUtils::InitLookUpTables(*m_vulkanDevice);

    m_descriptorSetLayoutCache = std::make_unique<VulkanCore::VDescriptorLayoutCache>(*m_vulkanDevice);


    m_uniformBufferManager = std::make_unique<VulkanUtils::VUniformBufferManager>(*m_vulkanDevice);


    m_rayTracingDataManager = std::make_unique<VulkanUtils::VRayTracingDataManager>(*m_vulkanDevice);

    m_effectsLibrary = std::make_unique<ApplicationCore::EffectsLibrary>(*m_vulkanDevice, *m_uniformBufferManager,
                                                                         *m_rayTracingDataManager, *m_descriptorSetLayoutCache);


    auto assetManger = std::make_unique<ApplicationCore::AssetsManager>(*m_vulkanDevice, *m_effectsLibrary);
    m_client->MountAssetsManger(std::move(assetManger));
    m_client->Init();
    m_uiContext = std::make_unique<VEditor::UIContext>(*m_vulkanDevice, *m_vulkanInstance, *m_windowManager, *m_client);


    m_frame = std::make_unique<Renderer::Frame>(*m_vulkanInstance, *m_vulkanDevice, *m_rayTracingDataManager, *m_uniformBufferManager,
                                                *m_effectsLibrary, *m_descriptorSetLayoutCache, *m_uiContext);


    m_frame->GetRenderContext()->defaultTexture       = m_client->GetAssetsManager().GetDummyTextureImage();
    m_frame->GetPostProcessingContext()->dummyTexture = m_client->GetAssetsManager().GetDummyTextureImage();

    m_frame->Init();
    m_uiContext->SetRenderingSystem(m_frame.get());


    //auto sponsa = m_client->GetGLTFLoader().LoadGLTFScene("/home/wpsimon09/Desktop/Models/sponza_scene/scene.gltf");
    ApplicationCore::ImportOptions importOptions{};

    if(std::filesystem::exists("cache/scene.gltf"))
    {
        m_client->GetGLTFLoader().LoadGLTFScene(m_client->GetScene(), "cache/scene.gltf", importOptions);
    }
    else
    {
        // build default scene
        m_client->GetScene().AddCubeToScene();
    }

    m_editor = std::make_unique<VEditor::Editor>(*m_uiContext);

    ApplicationCore::LoadClientSideConfig(*m_client, *m_uiContext);

    m_client->GetApplicationState().GetSceneUpdateFlags().rebuildAs = true;
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
    if(m_vulkanDevice->CurrentFrame >= GlobalVariables::MAX_FRAMES_IN_FLIGHT)
    {
        m_frame->GetTimelineSemaphore().CpuWaitIdle(EFrameStages::TransferFinish);
        m_vulkanDevice->GetTransferOpsManager().ClearResources();
    }
    if(m_vulkanDevice->CurrentFrame > 0)
    {
        m_vulkanDevice->GetTransferOpsManager().GetCommandBuffer().Reset();
        m_vulkanDevice->GetTransferOpsManager().StartRecording();
    }

    //=========================
    // Update the editor
    m_editor->SetVmaStatis(m_vulkanDevice->GetDeviceStatistics());
    m_editor->Update();

    //===============================================================
    // Update the scene with whatever data that were changed by editor
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

    //===================================
    // Send geometry data to the GPU
    m_vulkanDevice->GetMeshDataManager().UpdateGPU();

    //=====================================================
    // Update accelerations structures
    auto blasInputs = m_client->GetScene().GetBLASInputs();
    m_rayTracingDataManager->UpdateData(m_client->GetScene().GetSceneUpdateFlags(), blasInputs);

    m_client->GetApplicationState().SetIsWindowResized(m_windowManager->GetHasResized());
}

void Application::Render()
{

    m_client->Render(m_frame->GetRenderContext());

    m_editor->Render();

    // the frame update has to be here since editor render might change some stuff based on the UI alterations
    // this should be fixed with Command pattern or similar techinique
    m_vulkanDevice->GetTransferOpsManager().Sync();  // checks if CPU threads processing textuers are done
    m_frame->Update(m_client->GetApplicationState());

    if(m_frame->Render(m_client->GetApplicationState()))
    {
        m_frame->FinishFrame();
    }
}

void Application::PostRender()
{
    m_client->GetScene().Reset();
    m_client->GetApplicationState().Reset();
}

Application::~Application()
{
    ApplicationCore::SaveConfig(*m_client, *m_uiContext);

    if(!GlobalVariables::hasSessionBeenSaved)
    {
        try
        {
            for(const auto& entry : std::filesystem::directory_iterator(GlobalVariables::textureFolder))
            {
                std::filesystem::remove_all(entry.path());
            }
        }
        catch(std::exception& e)
        {
        }
    }
    m_vulkanDevice->GetDevice().waitIdle();
    m_frame->Destroy();
    m_effectsLibrary->Destroy();
    m_client->Destroy();
    m_uniformBufferManager->Destroy();
    VulkanCore::VSamplers::DestroyAllSamplers(*m_vulkanDevice);
    m_uiContext->Destroy();
    MathUtils::LookUpTables.ClearLoopUpTables();

    m_vulkanDevice->Destroy();
}
