#include "GLTFExporter.hpp"
#include <fastgltf/core.hpp>

ApplicationCore::GLTFExporter::GLTFExporter(ApplicationCore::Scene &scene, ApplicationCore::AssetsManager &assetsManager)
    :m_scene(scene), m_assetsManager(m_assetsManager)
{

}

void ApplicationCore::GLTFExporter::ExportScene(std::filesystem::path path)
{
    std::vector<std::shared_ptr<ApplicationCore::SceneNode>> sceneNodes;
    GetSceneNodes(m_scene.GetRootNode(), sceneNodes);
}

void ApplicationCore::GLTFExporter::GetAllSceneNodes(std::shared_ptr<SceneNode> &sceneNode, std::vector<std::shared_ptr<ApplicationCore::SceneNode>> &sceneNodes)
{
}

std::vector<std::shared_ptr<ApplicationCore::StaticMesh>> ApplicationCore::GLTFExporter::GetMeshes()
{
    return std::vector<std::shared_ptr<ApplicationCore::StaticMesh>>();
}

std::vector<std::shared_ptr<ApplicationCore::Material>> ApplicationCore::GLTFExporter::GetMaterials()
{
    return std::vector<std::shared_ptr<ApplicationCore::Material>>();
}
