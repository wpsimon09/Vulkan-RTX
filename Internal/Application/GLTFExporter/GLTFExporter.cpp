#include "GLTFExporter.hpp"
#include "Application/Logger/Logger.hpp"
#include <fastgltf/core.hpp>
#include <Application/Rendering/Scene/Scene.hpp>
#include <Application/Rendering/Scene/SceneNode.hpp>

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path, Scene& scene,
    const AssetsManager& assetsManager)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;
    GetAllSceneNodes(scene.GetRootNode(), sceneNodes);
    m_sceneNodeIndexCounter = 0;
    Utils::Logger::LogInfoClient("Parsed all scene nodes");
    fastgltf::Exporter exporter;
    fastgltf::Asset asset;

}

void ApplicationCore::GLTFExporter::GetAllSceneNodes(std::shared_ptr<SceneNode> sceneNode, std::vector<std::shared_ptr<ApplicationCore::SceneNode>> &sceneNodes)
{

    sceneNodes.push_back(sceneNode);
    sceneNode->GetSceneNodeMetaData().exportID = ++m_sceneNodeIndexCounter;
    for (auto& child : sceneNode->GetChildrenByRef())
    {
        GetAllSceneNodes(child, sceneNodes);
    }
}

std::vector<std::shared_ptr<ApplicationCore::StaticMesh>> ApplicationCore::GLTFExporter::GetMeshes()
{
    return std::vector<std::shared_ptr<ApplicationCore::StaticMesh>>();
}

std::vector<std::shared_ptr<ApplicationCore::Material>> ApplicationCore::GLTFExporter::GetMaterials()
{
    return std::vector<std::shared_ptr<ApplicationCore::Material>>();
}

fastgltf::Node ApplicationCore::GLTFExporter::ParseNode(std::shared_ptr<SceneNode> sceneNode)
{
    fastgltf::Node node;

}


