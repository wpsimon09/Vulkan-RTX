#ifndef GLTFEXPORTER_HPP
#define GLTFEXPORTER_HPP

#include <filesystem>
#include <vector>

namespace fastgltf
{
    struct Node;
}

namespace ApplicationCore {
    class StaticMesh;
    class SceneNode;
    class Scene;
    class AssetsManager;
    class Material;

    class GLTFExporter {
    public:
        GLTFExporter() = default;
    
        void ExportScene(std::filesystem::path path, Scene& scene, const AssetsManager& assetsManager);

    private:
        void GetAllSceneNodes(std::shared_ptr<SceneNode> sceneNode,std::vector<std::shared_ptr<ApplicationCore::SceneNode>>& sceneNodes);
        std::vector<std::shared_ptr<ApplicationCore::StaticMesh>> GetMeshes();
        std::vector<std::shared_ptr<ApplicationCore::Material>> GetMaterials();
        fastgltf::Node ParseNode(std::shared_ptr<SceneNode> sceneNode);

    private:
        int m_sceneNodeIndexCounter = 0;
        int m_meshIndexCounter = 0;
        int m_materialIndexCounter = 0;
        int m_imageIndexCounter = 0;
    };
}

#endif // GLTFEXPORTER_HPP