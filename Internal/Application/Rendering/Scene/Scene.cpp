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

namespace ApplicationCore {

    Scene::Scene(AssetsManager& assetsManager, Camera& camera): m_assetsManager(assetsManager), m_sceneStatistics(), m_camera(camera)
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
        m_root->Update();
    }

    void Scene::Render(VulkanUtils::RenderContext* ctx,std::shared_ptr<SceneNode> sceneNode )
    {
        if (sceneNode->HasMesh())
        {
            sceneNode->Render(m_assetsManager.GetEffectsLibrary(), ctx);
        }

        for (auto &child : sceneNode->GetChildrenByRef())
        {
            Render(ctx, child);
        }
    }

    void Scene::Reset()
    {
        m_sceneStatistics.Reset();
    }

    void Scene::RemoveNode(SceneNode* parent, std::shared_ptr<SceneNode> nodeToRemove) const
    {
        auto& children = parent->GetChildrenByRef();

        for (auto it = children.begin(); it != children.end();) {
            if (*it == nodeToRemove) {
                it->get()->ProcessNodeRemove();
                // in future when multiple nodes can be selected, this will account for shifting the list to the right
                it = children.erase(it);
                Utils::Logger::LogSuccessClient("Removed node from the scene graph");
                return;
            }else
            {
                ++it;
            }
        }
        Utils::Logger::LogErrorClient("Node not found");
    }


    void Scene::AddNode(std::shared_ptr<SceneNode> sceneNode) const
    {
        m_root->AddChild(sceneNode);
    }


    void Scene::BuildDefaultScene()
    {

        // Create materials
        Utils::Logger::LogSuccessClient("Default scene build");
    }

    void Scene::AddCubeToScene() const
    {
        auto obj = m_assetsManager.GetDefaultMesh(Cube);

        auto node = std::make_shared<SceneNode>(obj);
        node->SetName("Cube  ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }

    void Scene::AddSphereToScene() const
    {
        auto obj = m_assetsManager.GetDefaultMesh(Sphere);

        auto node = std::make_shared<SceneNode>(obj);
        node->SetName("Sphere  ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }

    void Scene::AddPlaneToScene() const
    {
        auto obj = m_assetsManager.GetDefaultMesh(Plane);

        auto node = std::make_shared<SceneNode>(obj);
        node->SetName("Plane ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }

    void Scene::AddSkyBox() const
    {
        auto obj = m_assetsManager.GetDefaultMesh(Cube);

        obj->SetMaterial(m_assetsManager.AddSkyBoxMaterial("Resources/HDRs/default.hdr"));

        auto node = std::make_shared<SkyBoxNode>(obj);

        node->SetName("Sky-Box ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }

    void Scene::AddDirectionalLight(LightStructs::DirectionalLight* directionalLightInfo)
    {
        if(m_sceneLightInfo.DirectionalLightInfo == nullptr){
            auto light = std::make_shared<DirectionLightNode>(m_sceneLightInfo,
                                                              m_assetsManager.GetEditorBilboardMesh(
                                                                  EEditorIcon::DirectionalLight), directionalLightInfo);
            light->SetName(light->GetMesh()->GetName());
            light->SetVisualisationMesh(m_assetsManager.GetDefaultMesh(EMeshGeometryType::Arrow));
            AddNode(light);
        }else{

            Utils::Logger::LogErrorClient("Directional light already exists !");
        }
    }

    void Scene::AddPointLight(LightStructs::PointLight* pointLightInfo)
    {
        if(m_sceneLightInfo.PointLightInfos.size() < 20){

            auto light = std::make_shared<PointLightNode>(m_sceneLightInfo, m_assetsManager.GetEditorBilboardMesh(EEditorIcon::PointLight), pointLightInfo);
            light->SetName(light->GetMesh()->GetName());

            AddNode(light);
        }else{
            Utils::Logger::LogErrorClient("Maximun number of point lights, 20,  reached !");
        }
    }

    void Scene::AddAreaLight(LightStructs::AreaLight* areaLightInfo)
    {
        if(m_sceneLightInfo.AreaLightInfos.size()<4){
            auto light = std::make_shared<AreaLightNode>(m_sceneLightInfo, m_assetsManager.GetEditorBilboardMesh(EEditorIcon::AreaLight), areaLightInfo);
            light->SetName(light->GetMesh()->GetName());
            light->SetVisualisationMesh(m_assetsManager.GetDefaultMesh(EMeshGeometryType::LinePlane));
            AddNode(light);
        }else{
            Utils::Logger::LogErrorClient("Maximun number of Area lights, 4,  reached !");
        }
    }


    void Scene::PreformRayCast(glm::vec2 mousePosition)
    {
        m_root->Deselect();
        m_selectedSceneNode = nullptr;
        if (mousePosition.x >= -1 && mousePosition.x <= 1 && mousePosition.y >= -1 && mousePosition.y <= 1)
        {
            Ray ray{};
            ray.origin = m_camera.GetPosition();
            ray.direction = m_camera.Deproject(mousePosition);
            ray.length = 20000.0f;

            Utils::Logger::LogSuccessClient("Ray constructed successfuly !");

            std::vector<std::shared_ptr<SceneNode>> hitNodes;
            for (auto& topNode : m_root->GetChildrenByRef())
            {
                topNode->PreformRayIntersectionTest(ray, hitNodes);
            };

            if (hitNodes.empty())
                return;

            auto closestNode = hitNodes[0];

            for (int i = 0; i < hitNodes.size(); i++)
            {
                Utils::Logger::LogInfo("Depth of the node is " + std::to_string(hitNodes[i]->GetDistanceFromCamera(m_camera.GetPosition())) );
                if (hitNodes[i]->GetDistanceFromCamera(m_camera.GetPosition()) < closestNode->GetDistanceFromCamera(m_camera.GetPosition()))
                    closestNode = hitNodes[i];
            }

            m_selectedSceneNode = closestNode;
            m_selectedSceneNode->Select();


        }else
        {
            Utils::Logger::LogErrorClient("Mouse is outside NDC");
        }

    }
} // ApplicationCore