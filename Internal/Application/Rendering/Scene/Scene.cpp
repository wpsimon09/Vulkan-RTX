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

    void Scene::BuildDefaultScene()
    {
        MaterialPaths red_brick;
        red_brick.DiffuseMapPath = "/home/wpsimon09/Desktop/Textures/red-brick/albedo.png";
        red_brick.ArmMapPath = "/home/wpsimon09/Desktop/Textures/red-brick/arm.png";
        red_brick.NormalMapPath = "/home/wpsimon09/Desktop/Textures/red-brick/normal.png";
        auto redBrickMaterial = m_assetsManager.GetMaterial(red_brick);

        MaterialPaths hangaBlack;
        hangaBlack.DiffuseMapPath = "/home/wpsimon09/Desktop/Textures/tiles/albedo.png";
        hangaBlack.ArmMapPath = "/home/wpsimon09/Desktop/Textures/tiles/arm.png";
        hangaBlack.NormalMapPath = "/home/wpsimon09/Desktop/Textures/tiles/normal.png";

        auto hangaBlackMaterial = m_assetsManager.GetMaterial(hangaBlack);

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
        auto floorMat = m_assetsManager.GetMaterial(hangaBlack);
        floorMat->GetMaterialDescription().values.diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);


        std::shared_ptr<VertexArray> sphereVertexArray = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_SPHERE);
        std::shared_ptr<VertexArray> postProcessVertexArray = m_assetsManager.GetVertexArrayForGeometryType(MESH_GEOMETRY_POST_PROCESS);


        // Create spheres
        auto floorSphere = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, hangaBlackMaterial, MESH_GEOMETRY_SPHERE);
        floorSphere->GetTransformations()->SetPosition(0.0f, -50.0f, 0.0f);
        floorSphere->GetTransformations()->SetScale(50.0f, 50.0f, 50.0f);

        auto sunSphere = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, sunMat, MESH_GEOMETRY_SPHERE);
        sunSphere->GetTransformations()->SetPosition(0.0f, 100.0f, -280.0f);
        sunSphere->GetTransformations()->SetScale(80.0f, 80.0f, 80.0f);

        auto sphere1 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, redMat, MESH_GEOMETRY_SPHERE);
        sphere1->GetTransformations()->SetPosition(-15.0f, -5.0f, -25.0f);
        sphere1->GetTransformations()->SetScale(4.0f, 4.0f, 4.0f);

        auto sphere2 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, greenMat, MESH_GEOMETRY_SPHERE);
        sphere2->GetTransformations()->SetPosition(12.0f, -8.0f, -30.0f);
        sphere2->GetTransformations()->SetScale(5.0f, 5.0f, 5.0f);

        auto sphere3 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, blueMat, MESH_GEOMETRY_SPHERE);
        sphere3->GetTransformations()->SetPosition(-7.0f, -3.0f, -15.0f);
        sphere3->GetTransformations()->SetScale(3.5f, 3.5f, 3.5f);

        auto sphere4 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, yellowMat, MESH_GEOMETRY_SPHERE);
        sphere4->GetTransformations()->SetPosition(8.0f, -6.0f, -20.0f);
        sphere4->GetTransformations()->SetScale(2.5f, 2.5f, 2.5f);

        auto sphere5 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, grayMat, MESH_GEOMETRY_SPHERE);
        sphere5->GetTransformations()->SetPosition(5.0f, -5.0f, -10.0f);
        sphere5->GetTransformations()->SetScale(3.0f, 3.0f, 3.0f);

        auto sphere6 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, purpleMat, MESH_GEOMETRY_SPHERE);
        sphere6->GetTransformations()->SetPosition(-10.0f, -5.0f, -5.0f);
        sphere6->GetTransformations()->SetScale(2.0f, 2.0f, 2.0f);

        auto sphere7 = std::make_shared<ApplicationCore::Mesh>(sphereVertexArray, redBrickMaterial, MESH_GEOMETRY_SPHERE);
        sphere7->GetTransformations()->SetPosition(3.0f, -4.0f, -18.0f);
        sphere7->GetTransformations()->SetScale(3.5f, 3.5f, 3.5f);

        auto rayTracerPlane = std::make_shared<ApplicationCore::Mesh>(postProcessVertexArray, blueMat,MESH_GEOMETRY_POST_PROCESS);
        rayTracerPlane->GetRenderingMetaData().bRasterPass = false;
        rayTracerPlane->GetRenderingMetaData().bRTXPass = true;



        m_root->AddChild(floorSphere);
        m_root->AddChild(sunSphere);

        m_root->AddChild(sphere1);  // Red Sphere
        m_root->AddChild(sphere2);  // Green Sphere
        m_root->AddChild(sphere3);  // Blue Sphere
        m_root->AddChild(sphere4);  // Yellow Sphere
        m_root->AddChild(sphere5);  // Gray Sphere
        m_root->AddChild(sphere6);  // Purple Sphere
        m_root->AddChild(sphere7);

        m_root->AddChild(rayTracerPlane);

        Utils::Logger::LogSuccessClient("Default scene build");
    }
} // ApplicationCore