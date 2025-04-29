//
// Created by wpsimon09 on 26/11/24.
//

#include "Scene.hpp"

#include <GLFW/glfw3.h>

#include "AreaLightNode.hpp"
#include "DirectionLightNode.hpp"
#include "PointLightNode.hpp"
#include "SceneNode.hpp"
#include "SkyBoxNode.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Enums/ClientEnums.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Material/SkyBoxMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhrHelpers.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"

namespace ApplicationCore {

void SceneData::AddEntry(const std::shared_ptr<ApplicationCore::SceneNode>& node) {
    if (node->HasMesh()) {
        auto& mesh = node->GetMesh();
        meshes.emplace_back(mesh);
        materials.emplace_back(mesh->GetMaterial());
        auto matTextures = mesh->GetMaterial()->EnumarateTexture();
        textures.insert(textures.end(), matTextures.begin(), matTextures.end());
    }
    nodes.emplace_back(node);
}

Scene::Scene(AssetsManager& assetsManager, Camera& camera)
    : m_assetsManager(assetsManager)
    , m_sceneStatistics()
    , m_camera(camera)
{
}

void Scene::Init()
{
    m_root = std::make_shared<SceneNode>();
    m_root->SetName("Root-Node");

    BuildDefaultScene();
}

void Scene::Update()
{
    if(!m_staticMeshes.empty())
    {
        return m_staticMeshes.clear();
    }
    m_root->Update(m_needsUpdate);
}

void Scene::Render(VulkanUtils::RenderContext* ctx, std::shared_ptr<SceneNode> sceneNode)
{
    if(sceneNode->HasMesh())
    {
        sceneNode->Render(m_assetsManager.GetEffectsLibrary(), ctx);
    }

    for(auto& child : sceneNode->GetChildrenByRef())
    {
        Render(ctx, child);
    }
}

void Scene::Reset()
{
    m_needsUpdate = false;
    m_needsRebuild = false;
    m_sceneStatistics.Reset();
}

void Scene::RemoveNode(SceneNode* parent, std::shared_ptr<SceneNode> nodeToRemove) const
{
    auto& children = parent->GetChildrenByRef();

    for(auto it = children.begin(); it != children.end();)
    {
        if(*it == nodeToRemove)
        {
            it->get()->ProcessNodeRemove();
            // in future when multiple nodes can be selected, this will account for shifting the list to the right
            it = children.erase(it);
            Utils::Logger::LogSuccessClient("Removed node from the scene graph");
            return;
        }
        else
        {
            ++it;
        }
    }
    Utils::Logger::LogErrorClient("Node not found");
}


void Scene::AddNode(std::shared_ptr<SceneNode> sceneNode)
{
    m_root->AddChild(m_sceneData, sceneNode);
    m_needsRebuild = true;
}

void Scene::EnumarateMeshes(std::vector<std::shared_ptr<SceneNode>>& outMeshes, std::shared_ptr<SceneNode> sceneNode)
{
    if(sceneNode->HasMesh())
    {
        outMeshes.emplace_back(sceneNode);
    }
    for(auto& child : sceneNode->GetChildrenByRef())
    {
        EnumarateMeshes(outMeshes, child);
    }
}

std::vector<VulkanCore::RTX::BLASInput> Scene::GetBLASInputs()
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> meshes;
    EnumarateMeshes(meshes, m_root);

    std::vector<VulkanCore::RTX::BLASInput> inputs;
    inputs.reserve(meshes.size());
    for(auto& m : meshes)
    {
        if(m->GetSceneNodeMetaData().VisibleInRayTracing && !m->IsLight())
        {
            auto& mesh = m->GetMesh();
            mesh->SetModelMatrix(m->m_transformation->GetModelMatrix());
            inputs.emplace_back(VulkanCore::RTX::StaticMeshToBLASInput(m->GetSceneNodeMetaData().ID, m->GetMesh(),
                                                                       m->m_transformation->GetModelMatrix()));
        }
    }


    return inputs;
}


void Scene::BuildDefaultScene()
{

    // Create materials
    Utils::Logger::LogSuccessClient("Default scene build");
}


void Scene::AddCubeToScene()
{
    auto obj = m_assetsManager.GetDefaultMesh(Cube);

    auto node = std::make_shared<SceneNode>(obj);
    node->SetName("Cube  ##" + VulkanUtils::random_string(5));
    AddNode(node);
}

void Scene::AddSphereToScene()
{
    auto obj = m_assetsManager.GetDefaultMesh(Sphere);

    auto node = std::make_shared<SceneNode>(obj);
    node->SetName("Sphere  ##" + VulkanUtils::random_string(5));
    AddNode(node);
}

void Scene::AddPlaneToScene()
{
    auto obj = m_assetsManager.GetDefaultMesh(Plane);

    auto node = std::make_shared<SceneNode>(obj);
    node->SetName("Plane ##" + VulkanUtils::random_string(5));
    AddNode(node);
}

void Scene::AddSkyBox(LightStructs::EnvLight* envLight)
{
    if(!m_currentSkyBox)
    {
        if(envLight != nullptr)
            m_currentSkyBox = m_assetsManager.AddSkyBoxMaterial(envLight->hdrPath);
        else
            m_currentSkyBox = m_assetsManager.AddSkyBoxMaterial("Resources/HDRs/default.hdr");

        auto obj = m_assetsManager.GetDefaultMesh(Cube);


        obj->SetMaterial(m_currentSkyBox);

        auto node = std::make_shared<SkyBoxNode>(m_sceneLightInfo, obj);

        node->SetName("Sky-Box ##" + VulkanUtils::random_string(5));

        AddNode(node);
    }
    else
    {
        Utils::Logger::LogError("Only one sky box can be active at time !");
    }
}

void Scene::AddDirectionalLight(LightStructs::DirectionalLight* directionalLightInfo)
{
    if(m_sceneLightInfo.DirectionalLightInfo == nullptr)
    {
        auto light = std::make_shared<DirectionLightNode>(
            m_sceneLightInfo, m_assetsManager.GetEditorBilboardMesh(EEditorIcon::DirectionalLight), directionalLightInfo);
        light->SetName(light->GetMesh()->GetName());
        light->SetVisualisationMesh(m_assetsManager.GetDefaultMesh(EMeshGeometryType::Arrow));
        AddNode(light);
    }
    else
    {

        Utils::Logger::LogErrorClient("Directional light already exists !");
    }
}

void Scene::AddPointLight(LightStructs::PointLight* pointLightInfo)
{
    if(m_sceneLightInfo.PointLightInfos.size() < 20)
    {

        auto light = std::make_shared<PointLightNode>(m_sceneLightInfo,
                                                      m_assetsManager.GetEditorBilboardMesh(EEditorIcon::PointLight), pointLightInfo);
        light->SetName(light->GetMesh()->GetName());

        AddNode(light);
    }
    else
    {
        Utils::Logger::LogErrorClient("Maximun number of point lights, 20,  reached !");
    }
}

void Scene::AddAreaLight(LightStructs::AreaLight* areaLightInfo)
{
    if(m_sceneLightInfo.AreaLightInfos.size() < 4)
    {
        auto light = std::make_shared<AreaLightNode>(m_sceneLightInfo,
                                                     m_assetsManager.GetEditorBilboardMesh(EEditorIcon::AreaLight), areaLightInfo);
        light->SetName(light->GetMesh()->GetName());
        light->SetVisualisationMesh(m_assetsManager.GetDefaultMesh(EMeshGeometryType::LinePlane));
        AddNode(light);
    }
    else
    {
        Utils::Logger::LogErrorClient("Maximun number of Area lights, 4,  reached !");
    }
}

void Scene::PreformRayCast(glm::vec2 mousePosition)
{
    m_root->Deselect();
    m_selectedSceneNode = nullptr;
    if(mousePosition.x >= -1 && mousePosition.x <= 1 && mousePosition.y >= -1 && mousePosition.y <= 1)
    {
        Ray ray{};
        ray.origin    = m_camera.GetPosition();
        ray.direction = m_camera.Deproject(mousePosition);
        ray.length    = 20000.0f;

        Utils::Logger::LogSuccessClient("Ray constructed successfuly !");

        std::vector<std::shared_ptr<SceneNode>> hitNodes;
        for(auto& topNode : m_root->GetChildrenByRef())
        {
            topNode->PreformRayIntersectionTest(ray, hitNodes);
        };

        if(hitNodes.empty())
            return;

        auto closestNode = hitNodes[0];

        for(int i = 0; i < hitNodes.size(); i++)
        {
            if(hitNodes[i]->GetDistanceFromCamera(m_camera.GetPosition()) < closestNode->GetDistanceFromCamera(m_camera.GetPosition()))
                closestNode = hitNodes[i];
        }

        m_selectedSceneNode = closestNode;
        m_selectedSceneNode->Select();
    }
    else
    {
        Utils::Logger::LogErrorClient("Mouse is outside NDC");
    }
}
bool Scene::NeedsUpdate()
{
    return m_needsUpdate;
}
bool Scene::NeedsRebuild() {
    return m_needsRebuild;
}
}  // namespace ApplicationCore