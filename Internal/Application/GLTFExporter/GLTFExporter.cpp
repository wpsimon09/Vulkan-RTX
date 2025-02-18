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
    fastgltf::Buffer m_vertexBuffer;
    fastgltf::Buffer m_indexBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackVertexBuffer()){
        vertices.insert(vertices.end(), buffer.data.begin(), buffer.data.end());        
    }

    for(auto& buffer : assetsManager.GetBufferAllocator().ReadBackIndexBuffers()){
        indices.insert(indices.end(), buffer.data.begin(), buffer.data.end());        
    }
    
    m_vertexBuffer.byteLength = vertices.size() * sizeof(Vertex);
    std::vector<std::byte> vertexBufferVector(reinterpret_cast<std::byte*>(vertices.data()), reinterpret_cast<std::byte*>(vertices.data()) + m_vertexBuffer.byteLength);
    if (reinterpret_cast<uintptr_t>(vertices.data()) % alignof(float) != 0) {
        Utils::Logger::LogErrorClient("Vertex buffer is not aligned to float");
    }
    m_vertexBuffer.data  = fastgltf::sources::Vector{vertexBufferVector};
    m_vertexBuffer.name = "Vertex buffers";

    m_indexBuffer.byteLength = indices.size() * sizeof(uint32_t);
    std::vector<std::byte> indexBufferVector(reinterpret_cast<std::byte*>(indices.data()), reinterpret_cast<std::byte*>(indices.data()) + m_indexBuffer.byteLength);
    m_indexBuffer.name = "Index buffers";
    m_indexBuffer.data = fastgltf::sources::Vector{indexBufferVector};

    //============================================
    // STORE VERTEX BUFFERS TO THE ASSET
    //============================================
    asset.buffers.resize(2);
    asset.buffers[0] = std::move(m_vertexBuffer);
    asset.buffers[1] = std::move(m_indexBuffer);

    //============================================
    // CREATE BUFFER VIEWS FOR VERTEX   
    //============================================
    fastgltf::BufferView vertexBufferView;
    vertexBufferView.bufferIndex = 0;
    vertexBufferView.byteLength = m_vertexBuffer.byteLength;
    vertexBufferView.byteOffset = 0;
    vertexBufferView.byteStride = sizeof(Vertex);
    vertexBufferView.name = "Vertex buffer view";
    vertexBufferView.target = fastgltf::BufferTarget::ArrayBuffer;
    asset.bufferViews.push_back(std::move(vertexBufferView));

    //============================================
    // CREATE BUFFER VIEWS FOR INDEX  
    //============================================
    fastgltf::BufferView indexBufferView;
    indexBufferView.bufferIndex = 1;
    indexBufferView.byteLength = m_indexBuffer.byteLength;
    indexBufferView.byteOffset = 0;
    indexBufferView.byteStride = sizeof(uint32_t);
    indexBufferView.name = "Index buffer view";
    indexBufferView.target = fastgltf::BufferTarget::ElementArrayBuffer;
    asset.bufferViews.push_back(std::move(indexBufferView));

    //============================================
    // CREATE POSITION ACCESSOR
    //============================================
    fastgltf::Accessor positionAccessor;
    positionAccessor.bufferViewIndex = 0;
    positionAccessor.byteOffset = offsetof(Vertex, position);
    positionAccessor.componentType = fastgltf::ComponentType::Float;
    positionAccessor.count = vertices.size();
    positionAccessor.type = fastgltf::AccessorType::Vec3;
    positionAccessor.name = "Position accessor";
    asset.accessors.push_back(std::move(positionAccessor));

    //============================================
    // CREATE POSITION ACCESSOR
    //============================================
    fastgltf::Accessor normalAccessor;
    normalAccessor.bufferViewIndex = 0;
    normalAccessor.byteOffset = offsetof(Vertex, normal);
    normalAccessor.componentType = fastgltf::ComponentType::Float;
    normalAccessor.count = vertices.size();
    normalAccessor.normalized = true;
    normalAccessor.name = "Normal accessor";
    normalAccessor.type = fastgltf::AccessorType::Vec3;
    asset.accessors.push_back(std::move(normalAccessor));


    //============================================
    // UV ACCESSOR
    //============================================
    fastgltf::Accessor uvAccessor;
    uvAccessor.bufferViewIndex = 0;
    uvAccessor.byteOffset = offsetof(Vertex, uv);
    uvAccessor.componentType = fastgltf::ComponentType::Float;
    uvAccessor.count = vertices.size();
    uvAccessor.normalized = true;
    uvAccessor.name = "UV accessor";
    uvAccessor.type = fastgltf::AccessorType::Vec2;
    asset.accessors.push_back(std::move(uvAccessor));

    //============================================
    // IDICES ACCESSOR
    //============================================
    fastgltf::Accessor indicesAccessor;
    indicesAccessor.bufferViewIndex = 1;
    indicesAccessor.byteOffset = 0;
    indicesAccessor.componentType = fastgltf::ComponentType::UnsignedInt;
    indicesAccessor.count = indices.size();
    indicesAccessor.name = "Indices accessor";
    indicesAccessor.type = fastgltf::AccessorType::Scalar;
    asset.accessors.push_back(std::move(indicesAccessor));

    ParseScene(scene.GetRootNode(), assetsManager, asset);

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
        primitive.attributes = {
            {"POSITION", 0},
            {"NORMAL", 1},
            {"TEXCOORD_0", 2}
        };
        primitive.indicesAccessor = 3;
        
        mesh.primitives.push_back(std::move(primitive));
        asset.meshes.push_back(std::move(mesh));
    }
    
    node.name = sceneNode->GetName();
    fastgltf::math::fmat4x4 modelMatrix;
    memcpy(&modelMatrix, &sceneNode->m_transformation->GetModelMatrix(), sizeof(modelMatrix));
    node.transform = modelMatrix;
    
    
    //m_sceneNodes.push_back({node});

    for (auto& child : sceneNode->GetChildrenByRef())
    {
        ParseScene(child, assetsManager, asset);
    }
}



