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

        // Create materials
        MaterialPaths redMaterialPath, greenMaterialPath, blueMaterialPath, yellowMaterialPath, grayMaterialPath;
        MaterialPaths purpleMaterialPath, orangeMaterialPath, sunMaterialPath, floorMaterialPath;

        // Red Material
        auto redMat = m_assetsManager.GetMaterial(redMaterialPath);
        redMat->GetMaterialDescription().values.diffuse = glm::vec4(0.9f, 0.0f, 0.0f, 1.0f);

        // Green Material
        auto greenMat = m_assetsManager.GetMaterial(greenMaterialPath);
        greenMat->GetMaterialDescription().values.diffuse = glm::vec4(0.0f, 0.9f, 0.0f, 1.0f);
        greenMat->GetMaterialDescription().values.roughness = 1.0f;

        // Blue Material
        auto blueMat = m_assetsManager.GetMaterial(blueMaterialPath);
        blueMat->GetMaterialDescription().values.diffuse = glm::vec4(0.0f, 0.0f, 0.9f, 1.0f);

        // Yellow Material
        auto yellowMat = m_assetsManager.GetMaterial(yellowMaterialPath);
        yellowMat->GetMaterialDescription().values.diffuse = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

        // Gray Material
        auto grayMat = m_assetsManager.GetMaterial(grayMaterialPath);
        grayMat->GetMaterialDescription().values.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

        // Purple Material
        auto purpleMat = m_assetsManager.GetMaterial(purpleMaterialPath);
        purpleMat->GetMaterialDescription().values.diffuse = glm::vec4(0.6f, 0.0f, 0.6f, 1.0f);

        // Orange Material
        auto orangeMat = m_assetsManager.GetMaterial(orangeMaterialPath);
        orangeMat->GetMaterialDescription().values.diffuse = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);

        // Sun Material (Emissive)
        auto sunMat = m_assetsManager.GetMaterial(sunMaterialPath);
        sunMat->GetMaterialDescription().values.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        sunMat->GetMaterialDescription().values.emissive_strength = glm::vec4(1.0f, 1.0f, 1.0f, 10.0f);

        // Floor Material
        auto floorMat = m_assetsManager.GetMaterial(floorMaterialPath);
        floorMat->GetMaterialDescription().values.diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

        // Create spheres
        auto floorSphere = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, floorMat, m_assetsManager);
        floorSphere->GetTransformations()->SetPosition(0.0f, 50.0f, 0.0f);
        floorSphere->GetTransformations()->SetScale(50.0f, 50.0f, 50.0f);

        auto sunSphere = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, sunMat, m_assetsManager);
        sunSphere->GetTransformations()->SetPosition(0.0f, 100.0f, -280.0f);
        sunSphere->GetTransformations()->SetScale(80.0f, 80.0f, 80.0f);

        auto sphere1 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, redMat, m_assetsManager);
        sphere1->GetTransformations()->SetPosition(-15.0f, 5.0f, -25.0f);
        sphere1->GetTransformations()->SetScale(4.0f, 4.0f, 4.0f);

        auto sphere2 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, greenMat, m_assetsManager);
        sphere2->GetTransformations()->SetPosition(12.0f, 8.0f, -30.0f);
        sphere2->GetTransformations()->SetScale(5.0f, 5.0f, 5.0f);

        auto sphere3 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, blueMat, m_assetsManager);
        sphere3->GetTransformations()->SetPosition(-7.0f, 3.0f, -15.0f);
        sphere3->GetTransformations()->SetScale(3.5f, 3.5f, 3.5f);

        auto sphere4 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, yellowMat, m_assetsManager);
        sphere4->GetTransformations()->SetPosition(8.0f, 6.0f, -20.0f);
        sphere4->GetTransformations()->SetScale(2.5f, 2.5f, 2.5f);

        auto sphere5 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, grayMat, m_assetsManager);
        sphere5->GetTransformations()->SetPosition(5.0f, 5.0f, -10.0f);
        sphere5->GetTransformations()->SetScale(3.0f, 3.0f, 3.0f);

        auto sphere6 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, purpleMat, m_assetsManager);
        sphere6->GetTransformations()->SetPosition(-10.0f, 5.0f, -5.0f);
        sphere6->GetTransformations()->SetScale(2.0f, 2.0f, 2.0f);

        auto sphere7 = std::make_shared<ApplicationCore::Mesh>(MESH_GEOMETRY_SPHERE, orangeMat, m_assetsManager);
        sphere7->GetTransformations()->SetPosition(3.0f, 4.0f, -18.0f);
        sphere7->GetTransformations()->SetScale(3.5f, 3.5f, 3.5f);

        m_root->AddChild(floorSphere);
        m_root->AddChild(sunSphere);

        m_root->AddChild(sphere1);  // Red Sphere
        m_root->AddChild(sphere2);  // Green Sphere
        m_root->AddChild(sphere3);  // Blue Sphere
        m_root->AddChild(sphere4);  // Yellow Sphere
        m_root->AddChild(sphere5);  // Gray Sphere
        m_root->AddChild(sphere6);  // Purple Sphere
        m_root->AddChild(sphere7);

        Utils::Logger::LogSuccessClient("Default scene build");
    }
} // ApplicationCore