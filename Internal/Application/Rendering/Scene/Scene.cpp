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
        m_root->m_transformation->SetRotations(1.0f, glm::degrees(sin(glfwGetTime())), 1.0F);
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
        auto cube1 =std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_CUBE, m_assetsManager);
        cube1->GetTransformations()->SetPosition(20.0f, -4.0f, 0.0f);
        cube1->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);

        auto cube2 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_CUBE, m_assetsManager);
        cube2->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);

        auto cube3 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_CUBE, m_assetsManager);
        cube3->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);
        cube3->GetTransformations()->SetPosition(-10.0f, -14.0f, 0.0f);

        auto plane = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, m_assetsManager);
        plane->GetTransformations()->SetScale(10.0f, 10.0f ,10.0f);
        plane->GetTransformations()->SetPosition(-10.0f, 24.0f, 0.0f);

        m_root->AddChild(cube1);
        m_root->AddChild(cube2);
        m_root->AddChild(cube3);
        m_root->AddChild(plane);
    }
} // ApplicationCore