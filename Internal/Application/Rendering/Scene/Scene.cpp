//
// Created by wpsimon09 on 26/11/24.
//

#include "Scene.hpp"

#include <GLFW/glfw3.h>

#include "SceneNode.hpp"
#include "Application/Enums/ClientEnums.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"

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

    void Scene::Render(std::vector<VulkanStructs::DrawCallData>& ctx,SceneNode& sceneNode )
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

    void Scene::BuildDefaultScene()
    {

        // cube 1
        auto obj1 =std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, m_assetsManager);
        obj1->GetTransformations()->SetPosition(0.0f, 10.0f, 0.0f);
        obj1->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);

        auto obj2 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, m_assetsManager);
        obj2->GetTransformations()->SetPosition(20.0f, -10.0f, 0.0f);
        obj2->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);

        auto obj3 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, m_assetsManager);
        obj3->GetTransformations()->SetPosition(40.0f, 10.0f, 0.0f);
        obj3->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);

        auto obj4 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, m_assetsManager);
        obj4->GetTransformations()->SetPosition(60.0f, -10.0f, 0.0f);
        obj4->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);

        m_root->AddChild(obj1);
        m_root->AddChild(obj2);
        m_root->AddChild(obj3);
        m_root->AddChild(obj4);
    }
} // ApplicationCore