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
    // CREATE BUFFERS 
    //========================================
    fastgltf::Buffer vertexBuffer;
    fastgltf::Buffer indexBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackVertexBuffer()){
        vertices.insert(vertices.end(), buffer.data.begin(), buffer.data.end());        
    }

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackIndexBuffers()){
        indices.insert(indices.end(), buffer.data.begin(), buffer.data.end());        
    }
    
    vertexBuffer.byteLength = vertices.size() * sizeof(Vertex);
    std::vector<std::byte> vertexBufferVector(reinterpret_cast<std::byte*>(vertices.data()), reinterpret_cast<std::byte*>(vertices.data()) + vertexBuffer.byteLength);
    if (reinterpret_cast<uintptr_t>(vertices.data()) % alignof(float) != 0) {
        Utils::Logger::LogErrorClient("Vertex buffer is not aligned to float");
    }
    
    vertexBuffer.data  = fastgltf::sources::Vector{vertexBufferVector};
    vertexBuffer.name = "Vertex buffers";

    indexBuffer.byteLength = indices.size() * sizeof(uint32_t);
    std::vector<std::byte> indexBufferVector(reinterpret_cast<std::byte*>(indices.data()), reinterpret_cast<std::byte*>(indices.data()) + indexBuffer.byteLength);
    indexBuffer.data = fastgltf::sources::Vector{indexBufferVector};

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



