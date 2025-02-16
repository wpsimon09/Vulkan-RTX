#include "GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
#include <fastgltf/core.hpp>
#include <Application/Rendering/Scene/Scene.hpp>
#include <Application/Rendering/Scene/SceneNode.hpp>

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path, Scene& scene,
    const AssetsManager& assetsManager)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;
    ParseScene(scene.GetRootNode());
    m_sceneNodeIndexCounter = 0;
    m_meshIndexCounter = 0;
    Utils::Logger::LogInfoClient("Parsed all scene nodes");

    fastgltf::Exporter exporter;
    fastgltf::Asset asset;

}

void ApplicationCore::GLTFExporter::ParseScene(std::shared_ptr<SceneNode> sceneNode, std::vector<Material>& materials)
{
    fastgltf::Node node{};
    fastgltf::Material material{};
    
    //===================================================
    // PARSE MESH DATA
    //===================================================
    if(sceneNode->HasMesh()){
        fastgltf::Mesh mesh;
        fastgltf::Primitive primitive;
        primitive.attributes;
    }
    
    node.name = sceneNode->GetName();
    fastgltf::math::fmat4x4 modelMatrix;
    memcpy(&modelMatrix, &sceneNode->m_transformation->GetModelMatrix(), sizeof(modelMatrix));
    node.transform = modelMatrix;

    
    m_sceneNodes.push_back({sceneNode, m_sceneNodeIndexCounter++});

    for (auto& child : sceneNode->GetChildrenByRef())
    {
        ParseScene(child);
    }
}



