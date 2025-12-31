//
// Created by wpsimon09 on 04/10/24.
//

#include "Client.hpp"

#include "Application/WindowManager/WindowManager.hpp"
#include "Logger/Logger.hpp"
#include "Rendering/Mesh/StaticMesh.hpp"
#include "VertexArray/VertexArray.hpp"
#include "Rendering/Transformations/Transformations.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Application/GLTFExporter/GLTFExporter.hpp"
#include "ApplicationState/ApplicationState.hpp"
#include "Rendering/Camera/Camera.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include <cassert>

#include "GLTFLoader/LoadSceneLights.hpp"
#include "Rendering/Scene/SceneNode.hpp"
#include "Rendering/Scene/Scene.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "AssetsManger/Utils/VTextureAsset.hpp"
#include "ApplicationState/ApplicationState.hpp"
#include "World/World.hpp"

Client::Client()
    : m_globalRenderingData2{}
{
    m_applicationState = std::make_unique<ApplicationCore::ApplicationState>();

    m_applicationState->pSetGlobalRenderingInfoData2(&m_globalRenderingData2);

    m_world = std::make_unique<ApplicationCore::World>(*m_applicationState);
}

void Client::Init()
{

    auto start = std::chrono::high_resolution_clock::now();

    Utils::Logger::LogInfoVerboseOnly("Creating camera...");
    m_camera = std::make_unique<ApplicationCore::Camera>();
    Utils::Logger::LogSuccessClient("Camera creatd");

    m_scene = std::make_unique<ApplicationCore::Scene>(*m_applicationState, *m_assetsManager, *m_camera);
    m_scene->Init();

    ApplicationCore::LoadSceneLights(*m_scene, GlobalVariables::lightInfoPath);


    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);


    Utils::Logger::LogSuccessClient("Client side initialized in: " + std::to_string(duration.count()) + "seconds");
}

const std::vector<std::reference_wrapper<ApplicationCore::StaticMesh>> Client::GetMeshes() const
{
    std::vector<std::reference_wrapper<ApplicationCore::StaticMesh>> result;
    result.reserve(m_meshes.size());
    for(auto& mesh : m_meshes)
    {
        result.push_back(std::ref(*mesh));
    }
    return result;
}

void Client::Render(VulkanUtils::RenderContext* ctx)
{
    ctx->projection = m_camera->GetProjectionMatrix();
    ctx->view       = m_camera->GetViewMatrix();
    m_scene->Render(ctx, m_scene->GetRootNode());
}

const void Client::MountAssetsManger(std::unique_ptr<ApplicationCore::AssetsManager> assetsManager)
{
    Utils::Logger::LogInfoClient("Mounting assets manger...");
    m_assetsManager = std::move(assetsManager);
    m_gltfLoader    = std::make_unique<ApplicationCore::GLTFLoader>(*m_assetsManager);
    m_gltfExporter  = std::make_unique<ApplicationCore::GLTFExporter>();
    assert(m_assetsManager);
    Utils::Logger::LogInfoClient("Mounted assets manager successfuly to the client");
}

const void Client::Destroy()
{
    Utils::Logger::LogInfoVerboseOnlyClient("Destroying client...");
    assert(m_assetsManager);
    Utils::Logger::LogInfoVerboseOnlyClient("Destroying assets manager...");
    m_assetsManager->DeleteAll();
    Utils::Logger::LogInfoVerboseOnlyClient("Assets manager destroyed");
    Utils::Logger::LogInfoVerboseOnlyClient("Destroyed client");
}

void Client::UpdateCamera(CameraUpdateInfo& cameraUpdateInfo)
{
    /*    // before camera is update we have the previous projection and view matrix
    m_globalRenderingData.viewPrevFrame = m_camera->GetViewMatrix();
    m_globalRenderingData.projPrevFrame = m_camera->GetProjectionMatrix();

    m_camera->Update(cameraUpdateInfo, m_scene->GetSceneUpdateFlags());

    m_globalRenderingData.proj        = m_camera->GetProjectionMatrix();
    m_globalRenderingData.view        = m_camera->GetViewMatrix();
    m_globalRenderingData.inverseView = m_camera->GetInverseViewMatrix();
    m_globalRenderingData.inverseProj = m_camera->GetinverseProjectionMatrix();

    m_globalRenderingData.screenSize = {GlobalVariables::RenderTargetResolutionWidth, GlobalVariables::RenderTargetResolutionHeight};
    m_globalRenderingData.viewParams2 = {glm::tan(m_camera->GetFOVRad() / 2.0), m_camera->GetAspectRatio(),
                                         m_camera->GetAparatureSize(), m_camera->GetCurrentCameraType()};

    m_globalRenderingData.viewParams      = glm::vec4(m_camera->GetImagePlaneDistance(), m_camera->GetFocalLength(),
                                                      m_camera->GetNearPlane(), m_camera->GetFarPlane());
    m_globalRenderingData.reccursionDepth = GlobalVariables::RenderingOptions::MaxRecursionDepth;
    m_globalRenderingData.raysPerPixel    = GlobalVariables::RenderingOptions::RaysPerPixel;
    m_globalRenderingData.cameraPosition  = glm::vec4(m_camera->GetPosition(), m_camera->GetFOVRad());

    m_globalRenderingData.rendererOutput    = m_applicationState->m_rendererOutput;
    m_globalRenderingData.rendererOutputRTX = m_applicationState->m_rtxRenderOutput;

    m_globalRenderingData.accumulateFrames = static_cast<bool>(m_applicationState->m_accumulateFrames);
    m_globalRenderingData.aoOcclusion      = static_cast<bool>(m_applicationState->m_ambientOcclusion);
    m_globalRenderingData.useComposition   = m_applicationState->m_composite;
    m_globalRenderingData.useReflection    = m_applicationState->m_rayTracedReflections;
 */
    //=======================================
    // New tightly packed format....
    // - atmosphere parameters are being filled in the ApplicationStateManager
    // - frame count and simialr are being updated during in the Update in Frame.hpp
    // - rest of the varibles have values filled in from here

    //=================================
    // Previous frame matrices
    m_globalRenderingData2.viewPrevFrame = m_camera->GetViewMatrix();
    m_globalRenderingData2.projPrevFrame = m_camera->GetProjectionMatrix();

    m_camera->Update(cameraUpdateInfo, m_scene->GetSceneUpdateFlags());

    //===========================================
    // Current frame projection and view matrices
    m_globalRenderingData2.proj        = m_camera->GetProjectionMatrix();
    m_globalRenderingData2.view        = m_camera->GetViewMatrix();
    m_globalRenderingData2.inverseView = m_camera->GetInverseViewMatrix();
    m_globalRenderingData2.inverseProj = m_camera->GetinverseProjectionMatrix();


    if(m_applicationState->GetAtmosphereParameters())
    {
        auto ap = m_applicationState->GetAtmosphereParameters();
        m_globalRenderingData2.atmosphereParams =
            glm::vec4(ap->groundAlbedo.w, ap->rayleighScattering.w, static_cast<bool>(ap->booleans.z), 0.0);
    }

    m_globalRenderingData2.cameraPosition = glm::vec4(m_camera->GetPosition(), m_camera->GetFOVRad());

    //========================
    // View parameters I
    m_globalRenderingData2.viewParams = glm::vec4(m_camera->GetImagePlaneDistance(), m_camera->GetFocalLength(),
                                                  m_camera->GetNearPlane(), m_camera->GetFarPlane());

    //========================
    // View parameters II
    m_globalRenderingData2.viewParams2 = glm::vec4(glm::tan(m_camera->GetFOVRad() / 2.0), m_camera->GetAspectRatio(),
                                                   m_camera->GetAparatureSize(), m_camera->GetCurrentCameraType());

    //========================
    // Rendering info I
    m_globalRenderingData2.renderingInfo =
        glm::vec4(GlobalVariables::RenderingOptions::RaysPerPixel, GlobalVariables::RenderingOptions::MaxRecursionDepth,
                  GlobalVariables::RenderTargetResolutionWidth, GlobalVariables::RenderTargetResolutionHeight);

    //========================
    // Rendering info II
    //m_globalRenderingData2.renderingInfo2.x - filled in during frame update in Fram::Update()
    m_globalRenderingData2.renderingInfo2.y = m_applicationState->m_rendererOutput;
    m_globalRenderingData2.renderingInfo2.z = m_applicationState->m_rtxRenderOutput;
    //m_globalRenderingData2.renderingInfo2.w - filled in during frame update in Fram::Update()

    //=====================
    // Features I
    m_globalRenderingData2.renderingFeatures.x = static_cast<bool>(m_applicationState->m_ambientOcclusion);
    // y - filled in during frame update,
    // z - filled in the RenderingOptions.hpp through ImGui::Checkbox
    m_globalRenderingData2.renderingFeatures.w = static_cast<bool>(m_applicationState->m_accumulateFrames);

    //=====================
    // Features  II
    m_globalRenderingData2.renderingFeatures2.x = m_applicationState->m_composite;
    m_globalRenderingData2.renderingFeatures2.y = m_applicationState->m_rayTracedReflections;
    m_globalRenderingData2.renderingFeatures2.z = m_applicationState->m_accumulateFrames;
    //w - is padding
}

void Client::UpdateClient(ClientUpdateInfo& lightUpdateInfo)
{
    // deprecated
    m_isRTXOn = lightUpdateInfo.isRTXon;

    lightUpdateInfo.Reset();
}

void Client::Update()
{
    m_scene->Update();
}
