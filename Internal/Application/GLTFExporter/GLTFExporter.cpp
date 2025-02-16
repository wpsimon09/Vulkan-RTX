#include "GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include <fastgltf/core.hpp>
#include <Application/Rendering/Scene/Scene.hpp>
#include <Application/Rendering/Scene/SceneNode.hpp>
#include <Application/AssetsManger/AssetsManager.hpp>

#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path, Scene& scene,
                                                AssetsManager& assetsManager)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;
    ParseScene(scene.GetRootNode(), assetsManager);
    m_sceneNodeIndexCounter = 0;
    m_meshIndexCounter = 0;
    Utils::Logger::LogInfoClient("Parsed all scene nodes");

    fastgltf::Exporter exporter;
    m_exportedScene = fastgltf::Asset();
    m_vertexBuffer = fastgltf::Buffer();


}

void ApplicationCore::GLTFExporter::ParseScene(std::shared_ptr<SceneNode> sceneNode, AssetsManager& assetsManager)
{
    fastgltf::Node node{};
    fastgltf::Material material{};
    
    //===================================================
    // PARSE MESH DATA
    //===================================================
    if(sceneNode->HasMesh()){
        fastgltf::Mesh mesh;
        fastgltf::Buffer meshBuffer;
        
        auto values = assetsManager.GetBufferAllocator().ReadBack(sceneNode->GetMesh()->GetMeshData()->vertexData);
        for (auto& vertex : values)
        {
            fastgltf::Buffer positionBuffer{};



            fastgltf::Accessor positionAccessor;
            fastgltf::Accessor normalAccessor;
            fastgltf::Accessor uvAccessor;

            //m_exportedScene.accessors.push_back();
            fastgltf::Accessor accessor;

            fastgltf::Attribute position;
            position.name = "POSITION";


           Utils::Logger::LogInfoClient("Parsed mesh");
        }
    }
    
    node.name = sceneNode->GetName();
    fastgltf::math::fmat4x4 modelMatrix;
    memcpy(&modelMatrix, &sceneNode->m_transformation->GetModelMatrix(), sizeof(modelMatrix));
    node.transform = modelMatrix;
    
    
    m_sceneNodes.push_back({node, m_sceneNodeIndexCounter++});

    for (auto& child : sceneNode->GetChildrenByRef())
    {
        ParseScene(child, assetsManager);
    }
}



