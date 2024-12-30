//
// Created by wpsimon09 on 26/11/24.
//

#include "Scene.hpp"

#include <random>
#include <GLFW/glfw3.h>

#include "SceneNode.hpp"
#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/Enums/ClientEnums.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Material/Material.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/VertexArray/VertexArray.hpp"

namespace ApplicationCore {

    Scene::Scene(AssetsManager& assetsManager): m_assetsManager(assetsManager), m_sceneStatistics()    {
       }

    void Scene::Init()
    {
        m_root = std::make_shared<SceneNode>();
        m_root->SetName("Root-Node");
        Utils::Logger::LogInfoVerboseOnly("Creating camera...");
        m_camera = std::make_unique<Camera>();
        Utils::Logger::LogSuccessClient("Camera created");

        BuildDefaultScene();
    }

    void Scene::Update()
    {
        m_root->Update();
    }

    void Scene::Render(VulkanStructs::RenderContext* ctx,std::shared_ptr<SceneNode> sceneNode )
    {
        if (sceneNode->HasMesh())
        {
            sceneNode->Render(ctx);
            m_sceneStatistics.drawCalls++;
            m_sceneStatistics.numberOfMeshes++;
        }

        for (auto &child : sceneNode->GetChildren2())
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
        auto& children = parent->GetChildren2();

        for (auto it = children.begin(); it != children.end();) {
            if (*it == nodeToRemove) {
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
        MaterialPaths blueMaterialPath;

        std::shared_ptr<VertexArray> sphereVertexArray = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_SPHERE);
        std::shared_ptr<VertexArray> postProcessVertexArray = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_POST_PROCESS);

        auto rayTracerPlane = std::make_shared<ApplicationCore::Mesh>(postProcessVertexArray, m_assetsManager.GetMaterial(blueMaterialPath),MESH_GEOMETRY_POST_PROCESS);
        rayTracerPlane->GetRenderingMetaData().bRasterPass = false;
        rayTracerPlane->GetRenderingMetaData().bRTXPass = true;

        AddNode(std::make_shared<SceneNode>(rayTracerPlane));

        Utils::Logger::LogSuccessClient("Default scene build");
    }

    void Scene::PrintSceneDatat(int depth, SceneNode& sceneNodes)
    {
        for (int i = 0; i < depth; i++)
        {
            std::cout << "\t";
        }
        std::cout << sceneNodes.GetName() << "\n";

        for (auto& child : sceneNodes.GetChildren())
        {
            PrintSceneDatat(depth + 1, child);
        }
    }

    void Scene::PrintSceneGraph()
    {
        int depth = 0;
        std::cout << "Root\n";
        std::cout << "|\n";

        PrintSceneDatat(depth, *m_root);
    }

    void Scene::AddCubeToScene() const
    {
        std::shared_ptr<VertexArray> objVAO = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_CUBE);
        auto obj = std::make_shared<ApplicationCore::Mesh>(objVAO, m_assetsManager.GetDummyMaterial(),MESH_GEOMETRY_CUBE);

        auto node = std::make_shared<SceneNode>(obj);
        node->SetName("Cube  ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }

    void Scene::AddSphereToScene() const
    {
        std::shared_ptr<VertexArray> objVAO = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_SPHERE);
        auto obj = std::make_shared<ApplicationCore::Mesh>(objVAO, m_assetsManager.GetDummyMaterial(),MESH_GEOMETRY_SPHERE);

        auto node = std::make_shared<SceneNode>(obj);
        node->SetName("Sphere  ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }

    void Scene::AddPlaneToScene() const
    {
        std::shared_ptr<VertexArray> objVAO = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_PLANE);
        auto obj = std::make_shared<ApplicationCore::Mesh>(objVAO, m_assetsManager.GetDummyMaterial(),MESH_GEOMETRY_PLANE);

        auto node = std::make_shared<SceneNode>(obj);
        node->SetName("Plane ##" + VulkanUtils::random_string(5));
        AddNode(node);
    }
} // ApplicationCore