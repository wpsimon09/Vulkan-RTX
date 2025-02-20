#include "GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include <fastgltf/core.hpp>
#include <Application/Rendering/Scene/Scene.hpp>
#include <Application/Rendering/Scene/SceneNode.hpp>
#include <Application/AssetsManger/AssetsManager.hpp>
#include <Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp>

#include "Application/Rendering/Mesh/StaticMesh.hpp"

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path, Scene& scene,
                                                AssetsManager& assetsManager)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;

    fastgltf::Asset asset;
    
    Utils::Logger::LogInfoClient("Parsing the scene");


    ParseBuffers(asset, assetsManager);
    ParseMaterial(asset, assetsManager);
    ParseScene(scene.GetRootNode(), assetsManager, asset);
    OrganiseScene(asset);
    CreateScene(asset);


    m_nodeCounter = 0;

    //=============================================
    // WRITE TO FILE
    //=============================================
    Utils::Logger::LogSuccessClient("Scene parsed successfuly");

    std::filesystem::path p = path;
    std::filesystem::path datapath = path / "data"; 
    fastgltf::FileExporter exporter;
    //exporter.setBufferPath(datapath);
    fastgltf::ExportOptions options = fastgltf::ExportOptions::None;
    auto result = exporter.writeGltfBinary(asset,path / "scene.glb");
    
    if(result != fastgltf::Error::None){
        Utils::Logger::LogSuccess("Scene saved to " + path.string());
        return;
    }else{
        Utils::Logger::LogErrorClient("Failed to save the scene:" + std::string(fastgltf::getErrorMessage(result)));
    }

    Utils::Logger::LogSuccessClient("Scene saved to " + path.string());
}

void ApplicationCore::GLTFExporter::ParseBuffers(fastgltf::Asset &asset, AssetsManager& assetsManager)
{
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
}

void ApplicationCore::GLTFExporter::ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager &assetsManager, fastgltf::Asset &asset)
{
    fastgltf::Material material{};
    
    //===================================================
    // PARSE MESH DATA
    //===================================================
    fastgltf::Node node{};
    if(sceneNode->HasMesh()){
        ParseMesh(asset, sceneNode->GetMesh());
        node.meshIndex = m_meshToIndex[sceneNode->GetMesh()];
    }else{
        node.meshIndex = std::nullopt;
    }

    node.name = sceneNode->GetName();
    
    fastgltf::math::fmat4x4 modelMatrix;
    memcpy(&modelMatrix,    &sceneNode->m_transformation->GetModelMatrix(), sizeof(modelMatrix));
    node.transform = modelMatrix;
    asset.nodes.push_back(std::move(node));

    m_nodes[sceneNode] = asset.nodes.size() - 1; 
    

    for (auto& child : sceneNode->GetChildrenByRef())
    {
        ParseScene(child, assetsManager, asset);
    }
}

void ApplicationCore::GLTFExporter::ParseMaterial(fastgltf::Asset &asset, AssetsManager &assetsManager)
{
    for (auto & mat : assetsManager.GetAllMaterials())
    {
        fastgltf::Material material;
        auto& matValues = mat->GetMaterialDescription().values;
        material.name = mat->GetMaterialName();
        material.alphaMode = mat->IsTransparent() ? fastgltf::AlphaMode::Blend : fastgltf::AlphaMode::Opaque;  
        material.pbrData.baseColorFactor = fastgltf::math::vec<float, 4>(matValues.diffuse.x, matValues.diffuse.y, matValues.diffuse.z, matValues.diffuse.w);
        material.pbrData.metallicFactor = matValues.metalness;
        material.pbrData.roughnessFactor = matValues.roughness;
        asset.materials.push_back(std::move(material));
        m_materialToIndex[mat] = asset.materials.size() - 1;
    }
    
}


void ApplicationCore::GLTFExporter::ParseMesh(fastgltf::Asset &asset, std::shared_ptr<StaticMesh> mesh)
{
        //auto& mesh = mesh;
    
        //============================================
        // CREATE BUFFER VIEWS FOR VERTEX   
        //============================================
        fastgltf::BufferView vertexBufferView;
        vertexBufferView.bufferIndex = 0;
        vertexBufferView.byteLength = mesh->GetMeshData()->vertexData.size;
        vertexBufferView.byteOffset = mesh->GetMeshData()->vertexData.offset;
        vertexBufferView.byteStride = sizeof(Vertex);
        vertexBufferView.name = "Vertex buffer view";
        vertexBufferView.target = fastgltf::BufferTarget::ArrayBuffer;
        asset.bufferViews.push_back(std::move(vertexBufferView));

        //============================================
        // CREATE BUFFER VIEWS FOR INDEX  
        //============================================
        fastgltf::BufferView indexBufferView;
        indexBufferView.bufferIndex = 1;
        indexBufferView.byteLength =  mesh->GetMeshData()->indexData.size;
        indexBufferView.byteOffset =  mesh->GetMeshData()->indexData.offset;
        indexBufferView.name = "Index buffer view";
        indexBufferView.target = fastgltf::BufferTarget::ElementArrayBuffer;
        asset.bufferViews.push_back(std::move(indexBufferView));

        //============================================
        // CREATE POSITION ACCESSOR
        //============================================
        fastgltf::Accessor positionAccessor;
        positionAccessor.bufferViewIndex = asset.bufferViews.size()-2;
        positionAccessor.byteOffset = offsetof(Vertex, position);
        positionAccessor.componentType = fastgltf::ComponentType::Float;
        positionAccessor.count = mesh->GetMeshData()->vertexData.size /sizeof(Vertex);
        positionAccessor.type = fastgltf::AccessorType::Vec3;
        positionAccessor.name = "Position accessor";

        std::pmr::vector<double>min{(double)mesh->GetMeshData()->bounds.min.x, (double)mesh->GetMeshData()->bounds.min.y, (double)mesh->GetMeshData()->bounds.min.z};
        std::pmr::vector<double>max{(double)mesh->GetMeshData()->bounds.max.x, (double)mesh->GetMeshData()->bounds.max.y, (double)mesh->GetMeshData()->bounds.max.z};

        positionAccessor.min = std::move(min);
        positionAccessor.max = std::move(max);
        asset.accessors.push_back(std::move(positionAccessor));
    

        //============================================
        // CREATE NORMAL ACCESSOR
        //============================================
        fastgltf::Accessor normalAccessor;
        normalAccessor.bufferViewIndex = asset.bufferViews.size() -2;
        normalAccessor.byteOffset = offsetof(Vertex, normal);
        normalAccessor.componentType = fastgltf::ComponentType::Float;
        normalAccessor.count = mesh->GetMeshData()->vertexData.size / sizeof(Vertex);
        normalAccessor.name = "Normal accessor";
        normalAccessor.type = fastgltf::AccessorType::Vec3;
        asset.accessors.push_back(std::move(normalAccessor));


        //============================================
        // UV ACCESSOR
        //============================================
        fastgltf::Accessor uvAccessor;
        uvAccessor.bufferViewIndex = asset.bufferViews.size() -2; // indices are the latest - 1  buffer pushed to this vector
        uvAccessor.byteOffset = offsetof(Vertex, uv);
        uvAccessor.componentType = fastgltf::ComponentType::Float;
        uvAccessor.count = mesh->GetMeshData()->vertexData.size / sizeof(Vertex);
        uvAccessor.name = "UV accessor";
        uvAccessor.type = fastgltf::AccessorType::Vec2;
        asset.accessors.push_back(std::move(uvAccessor));

        //============================================
        // IDICES ACCESSOR
        //============================================
        fastgltf::Accessor indicesAccessor;
        indicesAccessor.bufferViewIndex = asset.bufferViews.size()- 1; // indices is the latest buffer pushed to this vector
        indicesAccessor.byteOffset = 0;
        indicesAccessor.componentType = fastgltf::ComponentType::UnsignedInt;
        indicesAccessor.count = mesh->GetMeshData()->indexData.size / sizeof(uint32_t);
        indicesAccessor.name = "Indices accessor";
        indicesAccessor.type = fastgltf::AccessorType::Scalar;
        asset.accessors.push_back(std::move(indicesAccessor));


        fastgltf::Mesh m;
        fastgltf::Primitive primitive;
        primitive.materialIndex = m_materialToIndex[mesh->GetMaterial()];
        primitive.attributes = {
            {"POSITION", asset.accessors.size()-4},
            {"NORMAL", asset.accessors.size()-3},
            {"TEXCOORD_0", asset.accessors.size()-2}
        };
        primitive.indicesAccessor = asset.accessors.size() - 1;
        
        m.name = mesh->GetName();
        m.primitives.push_back(std::move(primitive));
        asset.meshes.push_back(std::move(m));


        m_meshToIndex[mesh] =asset.meshes.size() - 1;
}

void ApplicationCore::GLTFExporter::OrganiseScene(fastgltf::Asset &asset)
{
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {   
       if(!it->first->GetChildrenByRef().empty()){
            for(auto& child : it->first->GetChildrenByRef()){
                asset.nodes[it->second].children.push_back(m_nodes[child]);
            }
       }
    }    
}

void ApplicationCore::GLTFExporter::CreateScene(fastgltf::Asset &asset)
{
    asset.scenes.resize(1);
    asset.scenes[0].name = "Vulkan-RTX-saved-scene";
    // node at index 0 is allways the root
    asset.scenes[0].nodeIndices.push_back(0);
}
