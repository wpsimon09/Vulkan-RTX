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
        MaterialPaths materialPaths;
        materialPaths.DiffuseMapPath = "/home/wpsimon09/Desktop/Textures/hangar-black/albedo.png";
        materialPaths.ArmMapPath = "/home/wpsimon09/Desktop/Textures/hangar-black/arm.png";
        materialPaths.NormalMapPath = "/home/wpsimon09/Desktop/Textures/hangar-black/normal.png";

        auto hangarBlackMaterial = m_assetsManager.GetMaterial(materialPaths);

        MaterialPaths defaultMaterial;
        auto defaultMat = m_assetsManager.GetMaterial(defaultMaterial);
        defaultMat->GetMaterialDescription().values.diffuse = glm::vec4(0.8, 0.0, 0.1, 1.0);


        MaterialPaths defaultMaterial2;
        auto defaultMat2 = m_assetsManager.GetMaterial(defaultMaterial2);
        defaultMat2->GetMaterialDescription().values.diffuse = glm::vec4(0.0f, 0.2f, 1.0f,1.0f);
        defaultMat2->GetMaterialDescription().values.roughness = 0.6;

        MaterialPaths defaultMaterial3;
        auto defaultMat3 = m_assetsManager.GetMaterial(defaultMaterial3);
        defaultMat3->GetMaterialDescription().values.diffuse = glm::vec4(0.8f, 0.8f, 0.8f,1.0f);
        defaultMat3->GetMaterialDescription().values.roughness = 0.6;
        defaultMat3->GetMaterialDescription().values.metalness = 1.0;

        MaterialPaths defaultMaterial4;
        auto defaultMat4 = m_assetsManager.GetMaterial(defaultMaterial4);
        defaultMat4->GetMaterialDescription().values.roughness = 0.2;
        defaultMat4->GetMaterialDescription().values.metalness = 1.0;

        // cube 1
        auto obj1 =std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE,defaultMat4, m_assetsManager);
        obj1->GetTransformations()->SetPosition(0.0f, -3.0f, 0.0f);
        obj1->GetTransformations()->SetScale(3.0f, 3.0f ,3.0f);

        auto obj2 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE,defaultMat, m_assetsManager);
        obj2->GetTransformations()->SetPosition(6.0f, -4.0f, 0.0f);
        obj2->GetTransformations()->SetScale(4.0f, 4.0f ,4.0f);

        auto obj3 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE,hangarBlackMaterial, m_assetsManager);
        obj3->GetTransformations()->SetPosition(17.0f, -6.0f, 0.0f);
        obj3->GetTransformations()->SetScale(6.0f, 6.0f ,6.0f);

        auto obj4 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE,defaultMat3, m_assetsManager);
        obj4->GetTransformations()->SetPosition(32.0f, -8.0f, 0.0f);
        obj4->GetTransformations()->SetScale(8.0f, 8.0f ,8.0f);

        auto obj5 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE,defaultMat2, m_assetsManager);
        obj5->GetTransformations()->SetPosition(0.0f, 2.5f, 0.0f);
        obj5->GetTransformations()->SetScale(80.0f,2.0f ,80.0f);

        auto obj6 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_POST_PROCESS,defaultMat2, m_assetsManager);

        m_root->AddChild(obj1);
        m_root->AddChild(obj2);
        m_root->AddChild(obj3);
        m_root->AddChild(obj4);
        m_root->AddChild(obj5);
        m_root->AddChild(obj6);

        Utils::Logger::LogSuccessClient("Default scene build");
    }
} // ApplicationCore