#ifndef GLTFEXPORTER_HPP
#define GLTFEXPORTER_HPP

#include <filesystem>
#include <vector>

namespace ApplicationCore {
    class StaticMesh;
    class SceneNode;
    class Scene;
    class AssetsManager;
    class Material;
    
    class GLTFExporter {
    public:
        GLTFExporter(ApplicationCore::Scene& scene, ApplicationCore::AssetsManager& assetsManager);
    
        void ExportScene(std::filesystem::path path);
    private:
        void GetAllSceneNodes(std::shared_ptr<SceneNode>& sceneNode,std::vector<std::shared_ptr<ApplicationCore::SceneNode>>& sceneNodes);
        std::vector<std::shared_ptr<ApplicationCore::StaticMesh>> GetMeshes();
        std::vector<std::shared_ptr<ApplicationCore::Material>> GetMaterials();
    
        ApplicationCore::Scene& m_scene;
        ApplicationCore::AssetsManager& m_assetsManager;
    };
}

#endif // GLTFEXPORTER_HPP