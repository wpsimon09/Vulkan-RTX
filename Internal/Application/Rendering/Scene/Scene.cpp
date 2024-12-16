//
// Created by wpsimon09 on 26/11/24.
//

#include "Scene.hpp"

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

    Scene::Scene(AssetsManager& assetsManager): m_assetsManager(assetsManager)
    {
    }

    void Scene::Init()
    {
        m_root = std::make_unique<SceneNode>();
        Utils::Logger::LogInfoVerboseOnly("Creating camera...");
        m_camera = std::make_unique<Camera>();
        Utils::Logger::LogSuccessClient("Camera creatd");

        BuildDefaultScene();
    }

    void Scene::Update()
    {
        m_root->Update();
    }

    void Scene::Render(VulkanStructs::RenderContext* ctx,SceneNode& sceneNode )
    {
        if (sceneNode.HasMesh())
        {
            sceneNode.Render(ctx);
        }

        for (auto &child : sceneNode.GetChildren())
        {
            Render(ctx, child);
        }
    }

    void Scene::AddNode(std::unique_ptr<SceneNode> sceneNode)
    {
        m_root->AddChild(std::move(sceneNode));
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

        m_root->AddChild(rayTracerPlane);

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
} // ApplicationCore