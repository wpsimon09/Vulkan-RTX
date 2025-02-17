#include "GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include <fastgltf/core.hpp>
#include <Application/Rendering/Scene/Scene.hpp>
#include <Application/Rendering/Scene/SceneNode.hpp>
#include <Application/AssetsManger/AssetsManager.hpp>
#include <Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp>

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path, Scene& scene,
    AssetsManager& assetsManager)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;

    fastgltf::Asset asset;
    //========================================
    // READ BACK ALL BUFFERS 
    //========================================
    fastgltf::Buffer vertexBuffer;
    fastgltf::Buffer indexBuffer;

    std::vector<Vertex> vertices;

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackVertexBuffer()){
        vertices.insert(vertices.end(), buffer.data.begin(), buffer.data.end());        
    }
    memcpy(&vertexBuffer.data, vertices.data(), vertices.size() * sizeof(Vertex));


    ParseScene(scene.GetRootNode(), assetsManager, asset);
    m_sceneNodeIndexCounter = 0;
    m_meshIndexCounter = 0;
    Utils::Logger::LogInfoClient("Parsed all scene nodes");

    fastgltf::Exporter exporter;

}

void ApplicationCore::GLTFExporter::ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager& assetsManager,fastgltf::Asset& asset)
{
    
    fastgltf::Node node{};
    fastgltf::Material material{};
    
    
    //===================================================
    // PARSE MESH DATA
    //===================================================
    if(sceneNode->HasMesh()){
        fastgltf::Mesh mesh;
        fastgltf::Primitive primitive;
    }
    
    node.name = sceneNode->GetName();
    fastgltf::math::fmat4x4 modelMatrix;
    memcpy(&modelMatrix, &sceneNode->m_transformation->GetModelMatrix(), sizeof(modelMatrix));
    node.transform = modelMatrix;
    
    
    m_sceneNodes.push_back({node, m_sceneNodeIndexCounter++});

    for (auto& child : sceneNode->GetChildrenByRef())
    {
        ParseScene(child, assetsManager, asset);
    }
}



