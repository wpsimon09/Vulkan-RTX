//
// Created by wpsimon09 on 04/10/24.
//

#include "Client.hpp"

#include "Logger/Logger.hpp"
#include "Rendering/Mesh/Mesh.hpp"
#include "VertexArray/VertexArray.hpp"
#include "Rendering/Transformations/Transformations.hpp"
#include "Application/Rendering/Material/Material.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/GLTFLoader/GltfLoader.hpp"
#include "Rendering/Camera/Camera.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include <cassert>

#include "Rendering/Scene/SceneNode.hpp"
#include "Rendering/Scene/Scene.hpp"


Client::Client(): m_globalRenderingData()
{
}

void Client::Init() {

    auto start = std::chrono::high_resolution_clock::now();

    Utils::Logger::LogInfoVerboseOnly("Creating camera...");
    m_camera = std::make_unique<ApplicationCore::Camera>();
    Utils::Logger::LogSuccessClient("Camera creatd");

    m_scene = std::make_unique<ApplicationCore::Scene>(*m_assetsManager);
    m_scene->Init();


    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    Utils::Logger::LogSuccessClient("Client side initialized in: " +  std::to_string(duration.count()) + "seconds");
}

const std::vector<std::reference_wrapper<ApplicationCore::Mesh>> Client::GetMeshes() const {
    std::vector<std::reference_wrapper<ApplicationCore::Mesh>> result;
    result.reserve(m_meshes.size());
    for (auto &mesh : m_meshes) {
        result.push_back(std::ref(*mesh));
    }
    return result;
}

const void Client::Render(VulkanStructs::RenderContext* ctx)
{
    m_scene->Render(ctx);
}

const void Client::MountAssetsManger(std::unique_ptr<ApplicationCore::AssetsManager> assetsManager) {
    Utils::Logger::LogInfoClient("Mounting assets manger...");
    m_assetsManager = std::move(assetsManager);
    m_gltfLoader = std::make_unique<ApplicationCore::GLTFLoader>(*m_assetsManager);
    assert(m_assetsManager);
    Utils::Logger::LogInfoClient("Mounted assets manager successfuly to the client");
}

const void Client::Destroy() {
    Utils::Logger::LogInfoVerboseOnlyClient("Destroying client...");
    assert(m_assetsManager);
    Utils::Logger::LogInfoVerboseOnlyClient("Destroying assets manager...");
    m_assetsManager->DeleteAll();
    Utils::Logger::LogInfoVerboseOnlyClient("Assets manager destroyed");
    Utils::Logger::LogInfoVerboseOnlyClient("Destroyed client");
}

void Client::UpdateCamera(CameraUpdateInfo& cameraUpdateInfo)
{
    m_camera->Update(cameraUpdateInfo);

    m_globalRenderingData.proj = m_camera->GetProjectionMatrix();
    m_globalRenderingData.view = m_camera->GetViewMatrix();
    m_globalRenderingData.inverseView = m_camera->GetInverseViewMatrix();
    m_globalRenderingData.screenSize = m_camera->GetScreenSize();
    m_globalRenderingData.viewParams = glm::vec4(m_camera->GetCameraPlaneWidthAndHeight(), m_camera->GetNearPlane(),1.0f);
    m_globalRenderingData.playerPosition = glm::vec4(m_camera->GetPosition(),1.0f);
}

void Client::UpdateClient(ClientUpdateInfo& lightUpdateInfo)
{
    m_sunLightPosition.x += lightUpdateInfo.moveLightX;
    m_sunLightPosition.y += lightUpdateInfo.moveLightY;
    m_isRTXOn =             lightUpdateInfo.isRTXon;

    m_globalRenderingData.lightPosition = glm::vec4(m_sunLightPosition, 1.0f);

    lightUpdateInfo.Reset();
}

void Client::Update() {
    m_scene->Update();


}


